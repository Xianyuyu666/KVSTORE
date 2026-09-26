#include "kvstore/conc/thread_pool.h"

// submit 加锁，push，放锁，唤醒

ThreadPool::ThreadPool(size_t n_threads)
{
    for (size_t i = 0; i < n_threads; i++)
    {
        workers.emplace_back([this]()
                             { work_loop(); });

        // 等价写法
        // workers.emplace_back(&Thread::work_loop(),this);
    }
}

void ThreadPool::submit(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lk(mtx);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}

// 析构 加锁，修改stop，唤醒所有线程 等待所有线程退出
ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lk(mtx);
        stop = true;
    }
    cv.notify_all();
    for (auto &t : workers)
    {
        t.join();
    }
}