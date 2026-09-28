#include "kvstore/store/store.h"

void Store::Set(const std::string& key,const std::string& value){
    std::lock_guard<std::mutex> lk(mtx);
    map_[key] = value;
}

bool Store::Get(const std::string& key,std::string& value){
    std::lock_guard<std::mutex> lk(mtx);
    auto it = map_.find(key);
    if(it != map_.end()){
        value = it->second;
        return true;
    }
    return false;
}

bool Store::Del(const std::string& key){
    std::lock_guard<std::mutex> lk(mtx);
    auto it = map_.find(key);
    if(it != map_.end()){
        map_.erase(it);
        return true;
    }
    return false;
}


//[4B key_len][key][4B value_len][value]   ← 每条记录

bool Store::Save(std::string path){
    std::lock_guard<std::mutex> lk(mtx);
    std::ofstream f(path,std::ios::binary);
    if(!f.is_open())return false;
    for(auto& [key,value] : map_){
        uint32_t k_len = htonl(key.size());
        f.write(reinterpret_cast<const char*>(&k_len),sizeof(k_len));
        f.write(key.c_str(),key.size());
        uint32_t v_len = htonl(value.size());
        f.write(reinterpret_cast<const char*>(&v_len),sizeof(v_len));
        f.write(value.c_str(),value.size());
    }
    return true;
}

bool Store::Load(std::string path){
    std::lock_guard<std::mutex> lk(mtx);
    std::ifstream f(path,std::ios::binary);
    if(!f.is_open())return false;
    map_.clear();
    uint32_t k_len,v_len;
    while(f.read(reinterpret_cast<char *>(&k_len),4)){
        k_len = ntohl(k_len);
        std::string key(k_len,'\0');
        f.read(key.data(),k_len);
        f.read(reinterpret_cast<char *>(&v_len),4);
        v_len = ntohl(v_len);
        std::string value(v_len,'\0');
        f.read(value.data(),v_len);
        map_[key] = value;
    }
    return true;
}
