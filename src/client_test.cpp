#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

constexpr int PORT = 8888;
constexpr int chunk = 1024; // 一次发送1KB

void recv_frame(std::string &rp_msg, std::string &ans)
{
    while (ans.size() >= 4)
    {
        uint32_t len;
        memcpy(&len, ans.data(), 4);
        len = ntohl(len);
        rp_msg.append(ans.substr(4, len));
        ans.erase(0, 4 + len);
    }
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect");
        exit(1);
    }

    while(true){
        std::string msg;
        getline(std::cin,msg);
        if(msg == "quit")break;
        uint32_t len = htonl(msg.size());
        write(fd,&len,sizeof(len));
        write(fd,msg.data(),msg.size());
        uint32_t rp_len;
        read(fd,&rp_len,sizeof(rp_len));
        rp_len = ntohl(rp_len);
        std::string rp_msg(rp_len,'\0');
        read(fd,rp_msg.data(),rp_msg.size());
        std::cout << "服务器回复：" << rp_msg << std::endl;
    }
    close(fd);
    return 0;
}