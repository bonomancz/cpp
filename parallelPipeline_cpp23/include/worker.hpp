#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include "./threadSafeQueue.hpp"


class worker{
private:
    std::shared_ptr<threadSafeQueue<int>> generatorQueue, consumerQueue;
    std::jthread workerThread;
    std::atomic<bool> workerRunningFlag{true};
public:
    worker();
    ~worker();
    void start();
    void stop();
    void setGeneratorQueue(std::shared_ptr<threadSafeQueue<int>> gnrQueuePtr);
    void setConsumerQueue(std::shared_ptr<threadSafeQueue<int>> cnsQueuePtr);
    int getGeneratorQueueSize();
    int getConsumerQueueSize();
    int workerTask(int inputNo);
};