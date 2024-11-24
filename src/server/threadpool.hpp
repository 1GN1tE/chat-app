#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <string>
#include <iostream>

class ThreadPool
{
private:
    std::vector<std::thread> threads;        // Vector of worker threads
    std::queue<std::function<void()>> tasks; // Queue of tasks to execute
    std::mutex mutex;                        // Mutex for thread synchronization
    std::condition_variable condition;       // Condition variable for task availability
    bool stop;                               // Flag to stop the thread pool

private:
    void worker(); // Worker thread function

public:
    ThreadPool(int size);
    ~ThreadPool();

    template <typename Func, typename... Args>
    void enqueueTask(Func &&func, Args &&...args)
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            tasks.emplace([=]()
                          { func(args...); });
        }
        condition.notify_one();
    }
};
