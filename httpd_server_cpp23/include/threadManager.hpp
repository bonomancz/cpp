#include <iostream>
#include <string>
#include <format>
#include <memory>
#include <vector>
#include <thread>
#include <future>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <functional>
#include <typeinfo>
#include "threadSafeQueue.hpp"


class LogManager;

class ThreadManager{
private:
    std::shared_ptr<LogManager> logger;
    int threadCount{16};
    threadSafeQueue<std::function<void()>> threadPoolTaskQueue;
    std::atomic<bool> threadManagerRunningFlag{true};
    std::vector<std::jthread> threadPool;
public:
    ThreadManager(std::shared_ptr<LogManager> lgMgr);
    ~ThreadManager();
    void start();
    void stop();
    int getTaskQueueSize() const;

    template<typename F, typename... Args>
    auto submitThreadPoolTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>{
        using R = std::invoke_result_t<F, Args...>;
        auto taskPtr = std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<R> futureResult = taskPtr->get_future();
        threadPoolTaskQueue.emplace([taskPtr](){ (*taskPtr)(); });
        return futureResult;
    }
};