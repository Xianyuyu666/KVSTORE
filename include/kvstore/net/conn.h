#pragma once
#include <cstring>
#include <arpa/inet.h>
#include <cstdint>
#include <string>

constexpr int MAX_SIZE = 4096;  //一次read最大字节数

//生命周期由Reactor管理，accept时创建，连接关闭销毁
class Conn{
    public:
    Conn(int conn_fd);
    
    bool try_pop_frame(std::string& frame);
    void Read_append(const char * tmp,int size);
    void Write_append(const char * tmp,int size);
    std::string& Get_write_buf();

    private:
    int fd;
    std::string read_buf;
    std::string write_buf;
};