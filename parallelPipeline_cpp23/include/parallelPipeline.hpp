#pragma once
#include <iostream>
#include <thread>
#include <chrono>
#include "generator.hpp"
#include "worker.hpp"
#include "consumer.hpp"


class parallelPipeline{
private:
    std::atomic<bool> pipelineRunningFlag{true};
    generator gnr;
    worker wrk;
    consumer csm;
public:
    parallelPipeline();
    ~parallelPipeline();
    void start();
    void stop();
    bool getPipelineRunningFlag();
};