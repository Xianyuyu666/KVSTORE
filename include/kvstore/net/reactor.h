#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdint>

class Reactor
{

public:
    using EventCallback = std::function<void(uint32_t events)>;
    explicit Reactor(int max_events = 1024);
    ~Reactor();

    // 把fd加入事件循环，关注events(EPOLLIN,EPOLLOUT),之后有事件调用cb
    void add_fd(int fd, uint32_t events, EventCallback cb);

    // 修改fd关注的事件
    void mod_fd(int fd, uint32_t events);

    // 从事件循环移除fd
    void del_fd(int fd);

    // 阻塞运行事件循环
    void loop();

    // 退出
    void stop();

private:
    int epfd_;       // epoll实例句柄
    int max_events_; // 每次epoll_wait最多返回几个
    bool running_ = true;

    std::vector<struct epoll_event> events_;
    std::unordered_map<int, EventCallback> callbacks_;
}