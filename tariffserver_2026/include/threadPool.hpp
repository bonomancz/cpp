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

enum class workerRole : uint8_t {
    COMMON,
    NETWORK,
    DATABASE,
    LOG
};

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
    std::thread::id thread_id;
    std::atomic<workerRole> role{workerRole::COMMON};
    std::mutex workerMtx;
    std::condition_variable workerCv;
    static constexpr size_t circularQueueCapacity{8192};
    circularQueue<std::function<void()>, circularQueueCapacity> localWorkQueue;
    std::jthread workerThread;
};

class threadPool{
private:
    std::mutex mtx;
    std::deque<std::unique_ptr<threadPoolWorker>> workers;
    std::condition_variable fullWorkerCv;
    std::atomic<bool> threadPoolRunningFlag;
    std::atomic<bool> workersReadyToWork{false};
    std::atomic<int> activeThreadCount{0};
    std::atomic<size_t> globalTaskCount{0};
    int threadCount;
    std::shared_ptr<LogManager> logger;
public:
    threadPool(int threadCount, std::shared_ptr<LogManager>);
    threadPool(const threadPool&) = delete;
    threadPool& operator=(const threadPool&) = delete;
    ~threadPool();
    void start();
    void stop();
    bool isRunning() const;
    int getThreadPoolSize() const;
    int getActiveThreadCount() const;
    threadPoolWorker *getMinExpWorker(workerRole taskType);
    std::string getThreadPoolQueuesStatistics() const;
    bool getThreadPoolOccupancy(float maxOccupancy) const;
    int getThreadPoolPercentOccupancy() const;
    void setThreadCount(int threadCount);


    template<typename F, typename... Args>
    auto threadPoolSubmitTask(workerRole role, F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>{
        using R = std::invoke_result_t<F, Args...>;
        auto newTask = [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
            return f(std::move(args)...);
        };
        auto taskPtr = std::make_shared<std::packaged_task<R()>>(std::move(newTask));
        std::future<R> taskFuture = taskPtr->get_future();
        threadPoolWorker *targetWorker = nullptr;
        {
            std::unique_lock<std::mutex> poolLock(mtx);
            fullWorkerCv.wait(poolLock, [this](){
                return getThreadPoolOccupancy(1.0) || !threadPoolRunningFlag;
            });

            if(!threadPoolRunningFlag){
                throw std::runtime_error("threadPool::threadPoolSubmitTask(): ThreadPool stopped.");
            }
            targetWorker = this->getMinExpWorker(role);
        }
        {
            std::lock_guard<std::mutex> workerLock(targetWorker->workerMtx);
            targetWorker->localWorkQueue.enqueue([taskPtr](){ (*taskPtr)(); });
        }
        targetWorker->workerCv.notify_one();
        globalTaskCount.fetch_add(1, std::memory_order_relaxed);
        
        return taskFuture;
    }

};