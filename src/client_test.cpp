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

size_t send_frame(int fd, const std::string &body)
{
    uint32_t len = htonl(body.size());
    write(fd, &len, sizeof(len));
    return write(fd, body.data(), body.size());
}

std::string read_msg(int fd)
{
    uint32_t len;
    read(fd, &len, sizeof(len));
    len = ntohl(len);
    std::string ans(len, '\0');
    recv(fd, ans.data(), ans.size(),MSG_WAITALL);
    return ans;
}

int Conect(){
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
    return fd;
}

int main()
{
    int fd = Conect();
    char opt;
    std::cin >> opt;
    if (opt == 'a')
    {
        std::string msg(65536, 'a');
        send_frame(fd, "set name " + msg);
        auto ans = read_msg(fd);
        std::cout << "服务器回复：" << ans << std::endl;
        send_frame(fd, "get name");
        ans = read_msg(fd);
        std::cout << ans.size() << std::endl;
    }
    if (opt == 'b')
    {
        std::string msg(65527, 'a');
        size_t sent = 0;
        msg = "set name " + msg;
        uint32_t len = htonl(msg.size());
        write(fd, &len, sizeof(len));
        for (int i = 1; i <= 64; i++)
        {
            size_t s = write(fd, msg.data() + sent, 1024);
            sent += s;
        }
        auto ans = read_msg(fd);
        std::cout << "服务器回复:" << ans << std::endl;
        send_frame(fd, "get name");
        ans = read_msg(fd);
        std::cout << "size = " << ans.size() << std::endl;
    }
    if (opt == 'c')
    {
        std::string msg(65536,'a');
        msg = "set name " + msg;
        uint32_t len = htonl(msg.size());
        write(fd,&len,sizeof(len));
        std::this_thread::sleep_for(std::chrono::seconds(1));
        write(fd,msg.data(),msg.size());
        auto ans = read_msg(fd);
        std::cout << ">" << ans << std::endl;
        send_frame(fd,"get name");
        ans = read_msg(fd);
        std::cout << "size:" << ans.size() << std::endl;
    }
    if(opt == 'd'){
        std::string msg(65536, 'a');
        send_frame(fd, "set name " + msg);
        close(fd);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        fd = Conect();
        send_frame(fd, "get name");
        auto ans = read_msg(fd);
        std::cout << ans.size() << std::endl;
    }
    return 0;
}