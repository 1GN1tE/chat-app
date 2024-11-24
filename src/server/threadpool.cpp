#include "threadpool.hpp"

ThreadPool::ThreadPool(int size)
    : stop(false)
{
    for (int i = 0; i < size; ++i)
        threads.emplace_back(&ThreadPool::worker, this);
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &thread : threads)
        thread.join();
}

void ThreadPool::worker()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this]()
                           { return stop || !tasks.empty(); });

            if (stop && tasks.empty())
                return;

            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}