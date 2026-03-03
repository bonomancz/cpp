#include "threadPool.hpp"
#include "logManager.hpp"


threadPool::threadPool(int threadCount, LogManager &lgMgr) : 
    threadPoolRunningFlag{true},
    threadCount{threadCount},
    logger{lgMgr}
{}


threadPool::~threadPool(){ stop(); }


void threadPool::start(){
    try{
        for(size_t i = 0; i < threadCount; i++){
            auto &worker = workers.emplace_back();
            worker.workerThread = std::jthread([this, &worker](std::stop_token st){
                logger.debug("New Thread started in Thread Pool.");
                
                while(!st.stop_requested() && isRunning()){
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> workerLock(mtx);
                        cv.wait(workerLock, [this, &worker, &st](){ return !worker.localWorkQueue.isEmpty() || st.stop_requested() || !threadPoolRunningFlag; });
                        if(st.stop_requested()){ break; }
                        if(!worker.localWorkQueue.isEmpty()){
                            job = std::move(worker.localWorkQueue.dequeue());
                            fullWorkerCv.notify_one();
                            globalTaskCount.fetch_sub(1, std::memory_order_relaxed);
                        }
                    }
                    if(job){
                        // logger.debug(std::format("ThreadPool CORE(): A task arrived handled by Thread_ID: {}", std::this_thread::get_id()));
                        try{
                            activeThreadCounter lock(activeThreadCount);
                            job();
                        }catch(std::exception &ex){
                            logger.error(ex.what());
                        }
                    }
                }
                logger.debug(std::format("Thread stopped. (thread_id: {})", std::this_thread::get_id()));
            });
            worker.thread_id = worker.workerThread.get_id();
        }
        logger.debug("ThreadPool with workers started.");
    }catch(std::runtime_error &ex){
        std::cerr << "threadPool::start(): " << ex.what() << std::endl;
    }
}


void threadPool::stop(){
    {
        std::lock_guard<std::mutex> rfLock(mtx);
        if(!threadPoolRunningFlag){ return; }
        logger.debug("ThreadPool stopping.");
        threadPoolRunningFlag = false;
    }    
    for(auto &worker : workers){
        worker.workerThread.request_stop();
    }
    cv.notify_all();
    for(auto &worker : workers){
        if(worker.workerThread.joinable()){
            worker.workerThread.join();
        }
        // clear all worker local queues
        while(!worker.localWorkQueue.isEmpty()){
            try{
                worker.localWorkQueue.dequeue();
            }catch(const std::exception &ex){
                logger.error(std::format("Error while clearing worker local queue: {}", ex.what()));
            }
        }
    }
    workers.clear();
    logger.debug("ThreadPool stopped.");
}


threadPoolWorker *threadPool::getMinExpWorker(){
    threadPoolWorker *minExpWorker = nullptr;
    size_t minQueueSize = threadPoolWorker::circularQueueCapacity + 1;
    for(auto &worker : workers){
        size_t localQueueSize = worker.localWorkQueue.getSize();
        if(localQueueSize < minQueueSize && !worker.localWorkQueue.isFull()){
            minQueueSize = localQueueSize;
            minExpWorker = &worker;
        }
    }
    return minExpWorker;
}


bool threadPool::getThreadPoolOccupancy(float maxOccupancy) const {
    size_t current = globalTaskCount.load(std::memory_order_relaxed);
    return (static_cast<float>(current) / (threadCount * threadPoolWorker::circularQueueCapacity)) < maxOccupancy;
}


int threadPool::getThreadPoolPercentOccupancy() const {
    size_t current = globalTaskCount.load(std::memory_order_relaxed);
    return static_cast<int>((static_cast<float>(current) / (threadCount * threadPoolWorker::circularQueueCapacity)) * 100);
}


std::string threadPool::getThreadPoolQueuesStatistics() const {
    std::string stats;
    stats.append("\n\n");
    stats.append("     TRHEADPOOL STATISTICS - UTILIZATION: ");
    stats.append(std::to_string(getThreadPoolPercentOccupancy()));
    stats.append("%            ");
    stats.append("\n");
    stats.append("--------------------------------------------------------");
    stats.append("\n");
    for(const auto &worker : workers){
        stats.append(std::format("Thread_ID: {}, Local Queue Size: {}\n", worker.thread_id, worker.localWorkQueue.getSize()));
    }
    stats.append("--------------------------------------------------------");
    stats.append("\n\n");
    return stats;
}


bool threadPool::isRunning() const { return threadPoolRunningFlag; }
int threadPool::getThreadPoolSize() const { return workers.size(); }
int threadPool::getActiveThreadCount() const { return activeThreadCount.load(); }

activeThreadCounter::activeThreadCounter(std::atomic<int>& sharedCounterRef) : counter(sharedCounterRef) { counter++; }
activeThreadCounter::~activeThreadCounter(){ counter--; }
