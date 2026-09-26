#include "kvstore/conc/thread_pool.h"
#include <iostream>


int main(){
    ThreadPool pool((size_t)4);
    for(int i = 0;i < 100;i++){
        pool.submit([i](){
            std::cout << "线程 " << std::this_thread::get_id() << " 执行任务"
            << i << std::endl; 
        });
    }
}