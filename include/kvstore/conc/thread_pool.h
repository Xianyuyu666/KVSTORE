#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
public:
    explicit ThreadPool(size_t n_threads); // 创建n个工作线程
    ~ThreadPool();                         // 析构:停止并回收所有线程

    void submit(std::function<void()> task);

private:
    void work_loop()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [this]()
                    { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
            {
                return;
            }
            auto task = tasks.front();
            tasks.pop();
            lk.unlock();
            task();
        }
    }

    std::vector<std::thread> workers;        // 工作线程数组
    std::queue<std::function<void()>> tasks; // 任务队列
    std::mutex mtx;                          // 保护任务队列
    std::condition_variable cv;              // 条件变量
    bool stop = false;
};