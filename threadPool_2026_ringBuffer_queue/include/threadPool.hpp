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
#include <deque>
#include <chrono>
#include <functional>
#include "circularArrayDequeue.hpp"


class LogManager;

class activeThreadCounter{
private:
    std::atomic<int>& counter;
public:
    explicit activeThreadCounter(std::atomic<int>& sharedCounterRef);
    ~activeThreadCounter();
    activeThreadCounter(const activeThreadCounter&) = delete;
    activeThreadCounter& operator=(const activeThreadCounter&) = delete;
};

struct threadPoolWorker{
    std::jthread::id thread_id;
    static constexpr size_t circularQueueCapacity{8};
    circularQueue<std::function<void()>, circularQueueCapacity> localWorkQueue;
    std::jthread workerThread;
};

class threadPool{
private:
    std::mutex mtx;
    std::deque<threadPoolWorker> workers;
    std::condition_variable cv, fullWorkerCv;
    std::atomic<bool> threadPoolRunningFlag;
    std::atomic<int> activeThreadCount{0};
    std::atomic<size_t> globalTaskCount{0};
    int threadCount;
    LogManager &logger;
public:
    threadPool(int threadCount, LogManager &lgMgr);
    ~threadPool();
    void start();
    void stop();
    bool isRunning() const;
    int getThreadPoolSize() const;
    int getActiveThreadCount() const;
    threadPoolWorker *getMinExpWorker();
    std::string getThreadPoolQueuesStatistics() const;
    bool getThreadPoolOccupancy(float maxOccupancy) const;
    int getThreadPoolPercentOccupancy() const;


    template<typename F, typename... Args>
    auto threadPoolSubmitTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>{
        using R = std::invoke_result_t<F, Args...>;
        auto newTask = [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
            return f(std::move(args)...);
        };
        auto taskPtr = std::make_shared<std::packaged_task<R()>>(std::move(newTask));
        std::future<R> taskFuture = taskPtr->get_future();
        {
            std::unique_lock<std::mutex> queueLock(mtx);
            fullWorkerCv.wait(queueLock, [this](){
                return getThreadPoolOccupancy(1.0) || !threadPoolRunningFlag;
            });

            if(!threadPoolRunningFlag){
                throw std::runtime_error("threadPool::threadPoolSubmitTask(): ThreadPool stopped.");
            }
            this->getMinExpWorker()->localWorkQueue.enqueue([taskPtr](){ (*taskPtr)(); });
            globalTaskCount.fetch_add(1, std::memory_order_relaxed);
        }
        cv.notify_all();
        return taskFuture;
    }

};