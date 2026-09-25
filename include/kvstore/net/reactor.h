#include<unordered_map>
#include <functional>
#include <cstdint>
#include <sys/epoll.h>

constexpr int MAX_EVENTS = 1024;
using Callback_func = std::function<void(int fd,uint32_t event)>;

class Reactor{
    public:
    Reactor();

    void add_fd(int fd,uint32_t event,Callback_func cb);

    void loop();
    private:
    int epfd; //epoll实例
    epoll_event events[MAX_EVENTS];
    std::unordered_map<int,Callback_func>Callbacks;
};