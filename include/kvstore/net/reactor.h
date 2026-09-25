#pragma once
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <sys/epoll.h>

constexpr int MAX_EVENTS = 1024;
using Callback_func = std::function<void(int fd, uint32_t event)>;

class Reactor
{
public:
    Reactor();

    //增删改epoll
    void add_fd(int fd, uint32_t event, Callback_func cb);
    void del_fd(int fd);
    void mod_fd(int fd,uint32_t event);
    //事件循环
    void loop();


private:
    int epfd; // epoll实例
    epoll_event events[MAX_EVENTS];  //事件数组
    std::unordered_map<int, Callback_func> Callbacks;  //回调函数映射表
};