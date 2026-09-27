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