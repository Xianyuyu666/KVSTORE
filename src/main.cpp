#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/epoll.h>

constexpr int PORT = 8888;
constexpr int MAX_EVENTS = 1024;

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
    // epoll实例
    int epfd = epoll_create1(0);
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    std::cout << "服务器已启动..." << std::endl;

    // 事件循环
    epoll_event events[MAX_EVENTS];
    while (true)
    {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            uint32_t e = events[i].events;

            // 监听fd被唤醒->有新连接connect
            if (fd == listen_fd)
            {
                while (true)
                {
                    int conn_fd = accept(fd, nullptr, nullptr);
                    if (conn_fd == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        else
                        {
                            perror("accept error");
                            exit(1);
                        }
                    }
                    // 设置非阻塞
                    set_nonblocking(conn_fd);
                    // 打包事件加入事件循环
                    epoll_event cev{};
                    cev.events = EPOLLIN;
                    cev.data.fd = conn_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &cev);
                    std::cout << "新连接：fd = " << conn_fd << std::endl;
                }
                continue;
            }
            if (e & (EPOLLERR | EPOLLHUP))
            {
                close(fd);
                continue;
            }
            if (e & EPOLLIN)
            {
                char tmp[4096];
                while (true)
                {
                    ssize_t r = read(fd, tmp, sizeof(tmp));
                    if (r > 0)
                    {
                        write(fd, tmp, r);
                    }
                    else if (r == 0)
                    {
                        std::cout << "关闭：fd = " << fd << std::endl;
                        close(fd);
                        break;
                    }
                    else
                    {
                        if (errno == EAGAIN)
                            break;
                        close(fd);
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
