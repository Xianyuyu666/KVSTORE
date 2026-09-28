#pragma once

#include <unordered_map>
#include <mutex>
#include <string>
#include <fstream>
#include <arpa/inet.h>


class Store{
    private:
    std::unordered_map<std::string,std::string>map_;
    std::mutex mtx;  //保护map_
    public:
    void Set(const std::string& key,const std::string& value);
    bool Get(const std::string& key,std::string& value);
    bool Del(const std::string& key);
    bool Save(std::string path);
    bool Load(std::string path);
};