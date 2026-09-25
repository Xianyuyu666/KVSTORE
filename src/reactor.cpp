#include "kvstore/net/reactor.h"
#include <sys/epoll.h>

Reactor::Reactor(){
    epfd = epoll_create1(0);    
}

void Reactor::add_fd(int fd,uint32_t event,Callback_func cb){
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = event;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
    Callbacks[fd] = cb;
}

void Reactor::loop(){
    while(true){
        ssize_t n = epoll_wait(epfd,events,MAX_EVENTS,-1);
        for(int i = 0;i < n;i++){
            int fd = events[i].data.fd;
            uint32_t e = events[i].events;
            if(Callbacks.find(fd) != Callbacks.end()){
                Callbacks[fd](fd,e);
            }
        }
    }
}