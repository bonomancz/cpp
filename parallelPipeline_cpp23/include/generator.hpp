#pragma once
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include "./threadSafeQueue.hpp"


class generator{
private:
    std::jthread generatorWorker;
    std::atomic<bool> generatorRunningFlag{true};
    std::shared_ptr<threadSafeQueue<int>> generatorQueue = std::make_shared<threadSafeQueue<int>>();
public:
    generator();
    ~generator();
    int generatePseudoRand();
    void pushNewTasks(int count);
    void start();
    void stop();
    std::shared_ptr<threadSafeQueue<int>> getGeneratorQueue() const;
};