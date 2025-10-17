#pragma once
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>


template<typename T>
class threadSafeQueue{
private:
    std::queue<T> q;
    mutable std::mutex mtx;
    bool done = false;
    std::condition_variable cv;
public:
    threadSafeQueue(){}

    void push(T value){
        {
            std::lock_guard<std::mutex> lock(mtx);
            q.push(std::move(value));
        }
        cv.notify_one();
    }


    void emplace(T value){
        {
            std::lock_guard<std::mutex> lock(mtx);
            q.emplace(std::move(value));
        }
        cv.notify_one();
    }


    bool pop(T& output){
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&](){ return done || !q.empty(); });
            if(q.empty()){ return false; }
            output = std::move(q.front());
            q.pop();
        }
        return true;
    }


    void setDone(){
        {
            std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cv.notify_all();
    }


    size_t size() const{
        std::lock_guard<std::mutex> lock(mtx);
        return q.size();
    }


    bool empty() const{
        std::lock_guard<std::mutex> lock(mtx);
        return q.empty();
    }
};