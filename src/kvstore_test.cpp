#include "kvstore/store/store.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int Connect(){
    int fd = socket(AF_INET,SOCK_STREAM,0);
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);
    
    if(connect(fd,(sockaddr *)&addr,sizeof(addr)) == -1)return -1;
    
    return fd;
}

void send_frame(int fd,std::string body){
    uint32_t len = htonl(body.size());
    write(fd,&len,sizeof(len));
    write(fd,body.data(),body.size());
}

std::string read_frame(int fd){
    uint32_t len;
    recv(fd,&len,sizeof(len),MSG_WAITALL);
    len = ntohl(len);
    std::string rp(len,'\0');
    recv(fd,rp.data(),rp.size(),MSG_WAITALL);
    return rp;
}

int main() {
    int fd = Connect();
    if(fd == -1){
        perror("connect error");
        exit(1);
    }
    while(true){
        std::cout << ">";
        std::string msg;
        getline(std::cin,msg);
        if(msg == "quit")break;
        send_frame(fd,msg);
        std::string rp = read_frame(fd);
        std::cout << ">" << rp << std::endl;
    }
    close(fd);
    return 0;
}
