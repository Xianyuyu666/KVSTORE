#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

constexpr int PORT = 8888;
constexpr int chunk = 1024; //一次发送1KB

void recv_frame(std::string& rp_msg,std::string& ans){
    while(ans.size() >= 4){
        uint32_t len;
        memcpy(&len,ans.data(),4);
        len = ntohl(len);
        rp_msg.append(ans.substr(4,len));
        ans.erase(0,4 + len);
    }
}

int main(){
    int fd = socket(AF_INET,SOCK_STREAM,0);
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);

    if(connect(fd,(sockaddr *)&addr,sizeof(addr)) == -1){
        perror("connect");
        exit(1);
    }

    char opt;
    std::cin >> opt;
    std::string msg(65536,'A');
    uint32_t len = htonl(65536);
    //big测试
    if(opt == 'a'){
        std::cout << "big" << std::endl;
        write(fd,&len,sizeof(len));
        write(fd,msg.data(),msg.size());
        std::string ans,rp_msg;
        ans.resize(65540);
        recv(fd, ans.data(), 65540, MSG_WAITALL);  
        recv_frame(rp_msg,ans);
        std::cout << "收到回复字节数：（不包含长度头）" << rp_msg.size() << std::endl;
    }
    if(opt == 'b'){
        std::cout << "split" << std::endl;
        write(fd,&len,sizeof(len));
        for(int i = 0;i < 64;i++){
            write(fd,msg.data() + i * 1024,1024);
        }
        std::string ans,rp_msg;
        ans.resize(65540);
        recv(fd,ans.data(),65540, MSG_WAITALL);
        recv_frame(rp_msg,ans);
        std::cout << "收到回复字节数：" << rp_msg.size() << std::endl;
    }
    if(opt == 'c'){
        std::cout << "c" << std::endl;
        for(int i = 0;i < 64;i++){
            uint32_t L = htonl(1024);
            write(fd,&L,sizeof(L));
            write(fd,msg.data() + i * 1024,1024);
        }
        std::string ans,rp_msg;
        ans.resize(65792);
        recv(fd,ans.data(),65792,MSG_WAITALL);
        recv_frame(rp_msg,ans);
        std::cout << "收到回复字节数：" << rp_msg.size() << std::endl;
    }
    if(opt == 'd'){
        std::cout << "d" << std::endl;
        write(fd,&len,sizeof(len));
        close(fd);
    }
    return 0;
}