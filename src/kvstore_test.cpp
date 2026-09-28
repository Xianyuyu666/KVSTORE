#include "kvstore/store/store.h"
#include <iostream>

int main() {
    Store a;
    a.Set("name","zhangsan 张三");
    a.Set("x","!@#$%^&*()_+}{}:?><+-*//.,;'[]");
    if(a.Save("data.bin")){std::cout << "保存成功" << std::endl;}
    else std::cout << "保存失败" << std::endl;

    Store b;
    //不存在文件
    if(!b.Load("/ttt/sss.txt"))std::cout << "加载失败" << std::endl;

    if(b.Load("data.bin"))std::cout << "加载成功" << std::endl;
    else std::cout << "加载失败" << std::endl;

    std::string value;
    if(b.Get("name",value))std::cout << "name =" << value << std::endl;
    if(b.Get("x",value))std::cout << "x = " << value << std::endl;
    return 0;
}
