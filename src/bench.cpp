#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <netinet/tcp.h>
#include <netinet/in.h>

std::atomic<long> ok{0}, fail{0};

int Connect()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)); // 关 Nagle

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) == -1)
        return -1;
    return fd;
}

void write_all(int fd, const void *data, size_t len)
{ // 循环写到完
    const char *p = (const char *)data;
    while (len > 0)
    {
        ssize_t n = write(fd, p, len);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            return;
        }
        p += n;
        len -= n;
    }
}

void send_frame(int fd, std::string body)
{
    uint32_t len = htonl(body.size());
    std::string buf;
    buf.append((char *)&len, 4);           // 头
    buf.append(body);                      // body
    write_all(fd, buf.data(), buf.size()); // 一次写完
}

std::string read_frame(int fd)
{
    uint32_t len;
    ssize_t n = recv(fd, &len, 4, MSG_WAITALL);
    if (n <= 0)
        return ""; // 连接异常
    len = ntohl(len);
    if (len > 1 << 20)
        return ""; // 长度离谱：防坏数据
    std::string rp(len, '\0');
    n = recv(fd, rp.data(), rp.size(), MSG_WAITALL);
    if (n <= 0)
        return "";
    return rp;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    { // 参数检查
        return 1;
    }
    int conns = atoi(argv[1]);
    int per_conn = atoi(argv[2]);

    auto start = std::chrono::steady_clock::now(); // 单调时钟

    std::vector<std::thread> T;
    for (int i = 0; i < conns; i++)
    {
        T.emplace_back([per_conn]
                       {
            int fd = Connect();
            if(fd == -1){
                fail += (long)per_conn;
                return;
            }
            for(int j = 0;j < per_conn;j++){
                send_frame(fd,"get k");
                auto rp = read_frame(fd);
                if(rp == "NIL")ok++;
                else fail++;
            }
            close(fd); });
    }
    for (auto &t : T)
        t.join();

    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();
    long total = (long)conns * per_conn;
    std::cout << "QPS = " << (long)(total / sec)
              << ", 成功=" << ok.load()
              << ", 失败=" << fail.load() << std::endl;
    return 0;
}
