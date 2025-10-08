#pragma once
#include <iostream>
#include <sstream>
#include <format>
#include <thread>
#include <string>
#include <vector>
#include <future>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <queue>
#include <chrono>
#include <functional>


class threadPool{
private:
    std::queue<std::function<void()>> workQueue;
    std::mutex mtx;
    std::vector<std::jthread> workers;
    std::condition_variable cv;
    std::atomic<bool> threadPoolRunningFlag;
    int threadCount;
public:
    threadPool(int threadCount);
    ~threadPool();
    void start();
    void stop();
    bool getThreadPoolRunningFlag() const;
    int getThreadPoolSize() const;

    template<typename F, typename... Args>
    auto threadPoolSubmitTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>{
        using R = std::invoke_result_t<F, Args...>;
        auto taskPtr = std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<R> taskFuture = taskPtr->get_future();
        {
            std::unique_lock<std::mutex> queueLock(mtx);
            if(!threadPoolRunningFlag){ throw std::runtime_error("threadPool::threadPoolSubmitTask(): ThreadPool stopped."); }
            this->workQueue.emplace([taskPtr](){ (*taskPtr)(); });
        }
        cv.notify_one();
        return taskFuture;
    }
};