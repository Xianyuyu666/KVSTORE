#include "kvstore/net/conn.h"

Conn::Conn(int conn_fd) : fd(conn_fd) {};

void Conn::Read_append(const char * tmp,int size){
    read_buf.append(tmp,size);
}

void Conn::Write_append(const char * tmp,int size){
    write_buf.append(tmp,size);
}

std::string& Conn::Get_write_buf(){
    return write_buf;
}

std::string& Conn::Get_read_buf(){
    return read_buf;
}

bool Conn::try_pop_frame(std::string& frame){
    if(read_buf.size() < 4)return false;
    uint32_t len;
    memcpy(&len,read_buf.data(),4);
    len = ntohl(len);
    if(read_buf.size() >= 4 + len){
        frame = read_buf.substr(0,4 + len);
        read_buf.erase(0,4 + len);
        return true;
    }
    return false;
}