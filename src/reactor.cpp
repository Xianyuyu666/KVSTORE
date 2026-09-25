#include <kvstore/net/reactor.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

Reactor::Reactor(int max_events = 1024){
    epfd_ = epoll_create1(0);
}

void Reactor::loop(){
    while(running_){
        int n = epoll_wait(epfd_,events_.data(),max_events_,-1);
        for(int i = 0;i < n;i++){
            int fd = events_[i].data.fd;
            callbacks_[fd](events_[i].events);
        }
    }
}



