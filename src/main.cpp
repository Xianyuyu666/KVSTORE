#include <sstream>
#include <iostream>
#include <string>
#include "kvstore/store/store.h"

Store S;

std::string handle_frame(const std::string &frame)
{
    std::string msg = frame.substr(4);
    std::string opt,key,value,resp;
    std::stringstream ss(msg);
    ss >> opt >> key;
    ss.ignore();
    getline(ss,value);
    if(opt == "set"){
        S.Set(key,value);
        resp = "OK";
    }
    else if(opt == "get"){
        if(S.Get(key,value)){
            resp = value;
        }
        else resp = "NIL";
    }
    else if(opt == "del"){
        if(S.Del(key)){
            resp = "OK";
        }
        else resp = "NIL";
    }
    else{
        resp = "ERR 未知命令";
    }
    return resp;
}

int main(){
    std::cout << handle_frame("1111set name Jack Son") << std::endl;
    std::cout << handle_frame("1111get name") << std::endl;
    std::cout << handle_frame("1111del name") << std::endl;
    std::cout << handle_frame("1111hdel name") << std::endl;
}