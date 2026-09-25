#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>

int main(){
    int fd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);
    if(connect(fd,(sockaddr *)&addr,sizeof(addr)) != 0){
        std::cout << "连不上服务器" << std::endl;
        return 1;
    }
    std::string msg = "hello from client\n";
    send(fd,msg.data(),msg.size(),0);
    char buf[1024];
    ssize_t n = recv(fd,buf,sizeof(buf),0);
    if(n > 0)std::cout << "收到回复:" << std::string(buf,n);
    close(fd);
    return 0;
}