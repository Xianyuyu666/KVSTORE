#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/epoll.h>
#include "kvstore/net/reactor.h"

constexpr int PORT = 8888;

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
            T.add_fd(conn_fd,EPOLLIN,[](int fd,uint32_t event){
                (void)event;
                char msg[4096];
                while(true){
                    ssize_t r = read(fd,msg,sizeof(msg));
                    if(r > 0){
                        write(fd,msg,r);
                    }
                    else if(r == 0){
                        close(fd);
                        break;
                    }
                    else{
                        if(errno != EAGAIN){
                            close(fd);
                        }
                        break;
                    }
                }
            });
        } });
    std::cout << "服务器已启动..." << std::endl;
    T.loop();
    return 0;
}
