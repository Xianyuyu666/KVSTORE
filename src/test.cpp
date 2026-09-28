#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <sys/epoll.h>
#include <csignal>
#include <sys/eventfd.h>
#include "kvstore/util/logger.h"
#include "kvstore/net/conn.h"
#include "kvstore/net/reactor.h"
#include "kvstore/conc/thread_pool.h"
#include "kvstore/store/store.h"

constexpr int PORT = 8888;
const std::string DATA_PATH = "data.bin";

ThreadPool pool(4);
std::mutex resp_mtx;                                // 保护响应队列
std::queue<std::pair<int, std::string>> resp_queue; // 响应队列
std::unordered_map<int, Conn> Conns;                // 缓冲区
Store S;

void set_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

std::string handle_frame(const std::string &frame)
{
    std::string opt, key, value;
    std::stringstream ss(frame);
    ss >> opt;
    if ((opt == "get" || opt == "set" || opt == "del") && !(ss >> key))
        return "ERR 参数不足:缺少key";
    if (opt == "set")
    {
        ss.ignore();
        if (!getline(ss, value))
            return "ERR 参数不足:缺少value";
        S.Set(key, value);
        return "OK";
    }
    else if (opt == "get")
    {
        if (S.Get(key, value))
        {
            return value;
        }
        return "NIL";
    }
    else if (opt == "del")
    {
        if (S.Del(key))
        {
            return "OK";
        }
        return "NIL";
    }
    return "ERR 未知命令";
}

int main()
{
    signal(SIGPIPE, SIG_IGN);
    // 创建监听fd
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置地址复用
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(listen_fd, (sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 128);
    // 设置非阻塞
    set_nonblocking(listen_fd);

    Reactor T{};
    int evfd = eventfd(0, O_NONBLOCK);
    T.add_fd(evfd, EPOLLIN, [&T](int fd, uint32_t)
             {
        uint64_t r;
        read(fd,&r,sizeof(r));
        Log(LOG_INFO,"[fd=%d] 回调函数,事件类型：EPOLLIN",fd);
        std::vector<std::pair<int,std::string>>batch;
        {
            std::lock_guard<std::mutex> lk(resp_mtx);
            while(!resp_queue.empty()){
                batch.push_back(resp_queue.front());
                resp_queue.pop();
            }
        }
        for(auto& [fd,msg] : batch){
            if(Conns.count(fd)){
                Conns.at(fd).Write_frame(msg);
                Log(LOG_INFO,"[fd=%d] write_buf %d bytes rest",fd,Conns.at(fd).Get_write_buf().size());
                T.mod_fd(fd,EPOLLOUT | EPOLLIN);
            }
        } });
    T.add_fd(listen_fd, EPOLLIN, [&T, evfd](int fd, uint32_t event)
             {
        (void)event;
        Log(LOG_INFO,"[fd=%d] 回调函数,事件类型：%s",fd,T.EpollEventtoString(event).c_str());
        while (true)
        {
            int conn_fd = accept(fd, nullptr, nullptr);
            if (conn_fd == -1)
            {
                break;
            }
            Log(LOG_INFO, "new acception fd = %d", conn_fd);
            set_nonblocking(conn_fd);
            Conns.emplace(conn_fd, Conn(conn_fd));
            T.add_fd(conn_fd,EPOLLIN,[&T,evfd](int conn_fd,uint32_t event){
                Log(LOG_INFO,"[fd=%d] 回调函数,事件类型：%s",conn_fd,T.EpollEventtoString(event).c_str());
                // 读模块
                if (event & EPOLLIN)
                {
                    while (true)
                    {
                        char tmp[MAX_SIZE];
                        ssize_t r = read(conn_fd, tmp, MAX_SIZE);
                        if (r > 0)
                        {
                            Conns.at(conn_fd).Read_append(tmp, r);
                            Log(LOG_INFO, "[fd=%d] read %d bytes massage read_buf %zu bytes rest", conn_fd, r, Conns.at(conn_fd).Get_read_buf().size());
                        }
                        else if (r == 0)
                        {
                            //退出时处理缓存里残留数据
                            std::string frame;
                            while(Conns.at(conn_fd).try_pop_frame(frame)){
                                pool.submit([evfd,conn_fd,frame](){
                                    std::string resp = handle_frame(frame);
                                    {
                                        std::lock_guard<std::mutex> lk(resp_mtx);
                                        resp_queue.push({conn_fd,resp});
                                    }
                                    std::stringstream ss;
                                    ss << std::this_thread::get_id();
                                    Log(LOG_INFO,"[tid=%s] finished handle_frame",ss.str().c_str());
                                    uint64_t one = 1;
                                    write(evfd,&one,sizeof(one));
                                });
                            }
                            T.del_fd(conn_fd);
                            close(conn_fd);
                            Conns.erase(conn_fd);
                            Log(LOG_INFO, "[fd=%d] read done", conn_fd);
                            return;
                        }
                        else
                        {
                            if (errno == EAGAIN)
                            {
                                Log(LOG_INFO, "[fd=%d] read EAGAIN", conn_fd);
                                break;
                            }
                            T.del_fd(conn_fd);
                            close(conn_fd);
                            Conns.erase(conn_fd);
                            Log(LOG_ERROR, "[fd=%d] read error", conn_fd);
                            return;
                        }
                    }
                    std::string frame;
                    while (Conns.at(conn_fd).try_pop_frame(frame))
                    {
                        pool.submit([conn_fd,frame,evfd](){
                            std::string resp = handle_frame(frame);
                            {
                                std::lock_guard<std::mutex> lk(resp_mtx);
                                resp_queue.push({conn_fd,std::move(resp)});
                            }
                            std::stringstream ss;
                            ss << std::this_thread::get_id();
                            Log(LOG_INFO,"[tid=%s] finished handle_frame",ss.str().c_str());
                            uint64_t one = 1;
                            write(evfd,&one,sizeof(one));
                        });
                    }
                }
                //写模块
                if(event & EPOLLOUT){
                while (true)
                {
                    const void *data = Conns.at(conn_fd).Get_write_buf().data();
                    size_t len = Conns.at(conn_fd).Get_write_buf().size();
                    int s = write(conn_fd, data, len);
                    if (s > 0)
                    {
                        Conns.at(conn_fd).Get_write_buf().erase(0, s);
                        Log(LOG_INFO, "[fd=%d] write %d bytes write_buf %zu bytes rest", conn_fd, s, Conns.at(conn_fd).Get_write_buf().size());
                    }
                    if (s == 0)
                    {
                        close(conn_fd);
                        T.del_fd(conn_fd);
                        Conns.erase(conn_fd);
                        Log(LOG_INFO, "[fd=%d] write done", conn_fd);
                        return;
                    }
                    if (s < 0)
                    {
                        if (errno == EAGAIN)
                        {
                            Log(LOG_INFO, "[fd=%d] write EAGAIN write_buf = %zu bytes left", conn_fd, Conns.at(conn_fd).Get_write_buf().size());
                            break;
                        }
                        close(conn_fd);
                        T.del_fd(conn_fd);
                        Conns.erase(conn_fd);
                        return;
                    }
                    if (Conns.at(conn_fd).Get_write_buf().empty())
                    {
                        T.mod_fd(conn_fd, EPOLLIN);
                        break;
                    }
                }
                }
        });
        } });
    Log(LOG_INFO, "[fd=%d]server started on port %d", listen_fd, PORT);
    T.loop();
    return 0;
}
