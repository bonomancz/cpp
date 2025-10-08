#include "../include/threadPool.hpp"


threadPool::threadPool(int threadCount) : threadPoolRunningFlag(true), threadCount(threadCount){}


threadPool::~threadPool(){ stop(); }


void threadPool::start(){
    try{
        for(size_t i = 0; i < threadCount; i++){
            workers.emplace_back([this](std::stop_token st){
                while(!st.stop_requested() && getThreadPoolRunningFlag()){
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> workerLock(mtx);
                        cv.wait(workerLock, [this, &st](){ return !workQueue.empty() || st.stop_requested(); });
                        if(st.stop_requested()){ break; }
                        job = std::move(workQueue.front());
                        workQueue.pop();
                    }
                    // std::cout << "ThreadPool CORE(): A task arrived handled by Thread_ID: " << std::this_thread::get_id() << std::endl;
                    job();
                }
            });
        }
    }catch(std::runtime_error &ex){
        std::cerr << "threadPool::start(): " << ex.what() << std::endl;
    }
}


void threadPool::stop(){
    {
        std::lock_guard<std::mutex> rfLock(mtx);
        threadPoolRunningFlag = false;
    }
    for(auto &worker : workers){
        worker.request_stop();
    }
    cv.notify_all();
    workers.clear();
    {
        std::lock_guard<std::mutex> queueLock(mtx);
        std::queue<std::function<void()>> emptyQueue;
        std::swap(workQueue, emptyQueue);
    }
}


bool threadPool::getThreadPoolRunningFlag() const { return threadPoolRunningFlag; }
int threadPool::getThreadPoolSize() const { return workers.size(); }
