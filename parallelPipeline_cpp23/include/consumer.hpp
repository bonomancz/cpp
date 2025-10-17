#pragma once
#include <iostream>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "threadSafeQueue.hpp"


class consumer{
private:
    std::jthread consumerWorker;
    std::atomic<bool> consumerRunningFlag{true};
    std::mutex consumerMtx;
    std::condition_variable consumerCv;
    std::shared_ptr<threadSafeQueue<int>> consumerQueue = std::make_shared<threadSafeQueue<int>>();
public:
    consumer();
    ~consumer();
    void start();
    void stop();
    std::shared_ptr<threadSafeQueue<int>> getConsumerQueue() const;
};