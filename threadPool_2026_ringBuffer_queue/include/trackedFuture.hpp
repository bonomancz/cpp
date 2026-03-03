#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <future>


template<typename T>
class trackedFuture{
private:
        int id;
        std::string description;
        std::chrono::steady_clock::time_point start;
        std::future<T> tf;
public:
        trackedFuture(int id, const std::string &description, std::chrono::steady_clock::time_point steadyClockStart, std::future<T> taskFuture) : 
                id(id), 
                description(description), 
                start(steadyClockStart), 
                tf(std::move(taskFuture)){}
        int getId() const { return id; };
        std::string getDescription() const{ return description; }
        std::future<T> getTrackedFuture() { return std::move(tf); }
        bool isReady() { return tf.valid() && tf.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }
        template<typename Duration = std::chrono::microseconds>
        long long int getElapsedTime() const noexcept { return std::chrono::duration_cast<Duration>(std::chrono::steady_clock::now() - start).count(); }
};