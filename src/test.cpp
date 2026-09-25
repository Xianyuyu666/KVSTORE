#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/epoll.h>
#include "kvstore/net/conn.h"
#include "kvstore/net/reactor.h"

constexpr int PORT = 8888;

std::unordered_map<int, Conn> Conns;

void set_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

int main()
{
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
    T.add_fd(listen_fd, EPOLLIN, [&T](int fd, uint32_t event)
             {
                (void)event;
        while(true){
            int conn_fd = accept(fd,nullptr,nullptr);
            if(conn_fd == -1){
                break;
            }
            std::cout << "新连接：fd = " << conn_fd << std::endl;
            set_nonblocking(conn_fd);
            Conns.emplace(conn_fd,Conn(conn_fd));
            T.add_fd(conn_fd,EPOLLIN,[&T](int conn_fd,uint32_t event){
                //读模块
                if(event & EPOLLIN){
                    while(true){
                        char tmp[MAX_SIZE];
                        ssize_t r = read(conn_fd,tmp,MAX_SIZE);
                        if(r > 0){
                            Conns.at(conn_fd).Read_append(tmp,r);
                        }
                        else if(r == 0){
                            T.del_fd(conn_fd);
                            close(conn_fd);
                            Conns.erase(conn_fd);
                            return;
                        }
                        else{
                            if(errno == EAGAIN)break;
                            T.del_fd(conn_fd);
                            close(conn_fd);
                            Conns.erase(conn_fd);
                            return;
                        }
                    }
                    std::string frame;
                    while(Conns.at(conn_fd).try_pop_frame(frame)){
                        Conns.at(conn_fd).Write_append(frame.data(),frame.size());
                    }
                    if(Conns.at(conn_fd).Get_write_buf().size()){
                        T.mod_fd(conn_fd,EPOLLOUT | EPOLLIN);
                    }
                }
                //写模块
                if(event & EPOLLOUT){
                    while(true){
                        int s = write(conn_fd,Conns.at(conn_fd).Get_write_buf().data(),Conns.at(conn_fd).Get_write_buf().size());
                        if(s > 0){
                            Conns.at(conn_fd).Get_write_buf().erase(0,s);
                        }
                        if(s == 0){
                            close(conn_fd);
                            T.del_fd(conn_fd);
                            Conns.erase(conn_fd);
                            return;
                        }
                        if(s < 0){
                            if(errno == EAGAIN)break;
                            close(conn_fd);
                            T.del_fd(conn_fd);
                            Conns.erase(conn_fd);
                            return;
                        }
                        if(Conns.at(conn_fd).Get_write_buf().empty()){
                            T.mod_fd(conn_fd,EPOLLIN);
                            break;
                        }
                    }
                }
            });
        } });
    std::cout << "服务器已启动..." << std::endl;
    T.loop();
    return 0;
}
