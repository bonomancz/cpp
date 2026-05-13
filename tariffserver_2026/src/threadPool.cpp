#include "threadPool.hpp"
#include "logManager.hpp"


threadPool::threadPool(int threadCount, std::shared_ptr<LogManager> lgMgr) : 
    threadPoolRunningFlag{true},
    threadCount{threadCount},
    logger{lgMgr}
{}


threadPool::~threadPool(){ stop(); }


void threadPool::start(){
    try{
        if(threadCount < 4){ throw std::invalid_argument("threadPool::start(): Thread count must be at least 4."); }
        for(size_t i = 0; i < threadCount; i++){
            auto &worker = workers.emplace_back(std::make_unique<threadPoolWorker>());
            if(i == 0){ worker->role = workerRole::NETWORK; } // selecting expert tasks workers
            else if(i == 1){ worker->role = workerRole::DATABASE; }
            else if(i == 2){ worker->role = workerRole::LOG; }
            else{ worker->role = workerRole::COMMON; }
            worker->workerThread = std::jthread([this, worker = worker.get()](std::stop_token st){
                worker->thread_id = std::this_thread::get_id();

                // BARRIER - waiting for all threads are ready
                while(!workersReadyToWork.load(std::memory_order_acquire) && !st.stop_requested() && isRunning()){
                    std::this_thread::yield();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                logger->debug("New Thread started in Thread Pool.");
                
                while(!st.stop_requested() && isRunning()){
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> workerLock(worker->workerMtx);
                        worker->workerCv.wait(workerLock, [this, worker, &st](){ return !worker->localWorkQueue.isEmpty() || st.stop_requested() || !threadPoolRunningFlag; });
                        if(st.stop_requested()){ break; }
                        if(!worker->localWorkQueue.isEmpty()){
                            job = std::move(worker->localWorkQueue.dequeue());
                            fullWorkerCv.notify_one();
                            globalTaskCount.fetch_sub(1, std::memory_order_relaxed);
                        }
                    }
                    if(job){
                        try{
                            activeThreadCounter lock(activeThreadCount);
                            job();
                        }catch(std::exception &ex){
                            logger->error(ex.what());
                        }
                    }
                }
                logger->debug(std::format("Thread stopped. (thread_id: {})", worker->thread_id));
            });
        }

        workersReadyToWork.store(true, std::memory_order_release); // all workers ready

        logger->debug("ThreadPool started with workers.");
    }catch(std::exception &ex){
        std::cerr << "threadPool::start(): " << ex.what() << std::endl;
    }
}


void threadPool::stop(){
    {
        std::lock_guard<std::mutex> rfLock(mtx);
        if(!threadPoolRunningFlag){ return; }
        threadPoolRunningFlag = false;
    }    
    for(auto &worker : workers){
        worker->workerThread.request_stop();
        {
            std::lock_guard<std::mutex> workerLock(worker->workerMtx);
            worker->workerCv.notify_all();
        }
    }
    fullWorkerCv.notify_all();
    for(auto &worker : workers){
        if(worker->workerThread.joinable()){
            worker->workerThread.join();
        }
        // clear all worker local queues
        while(!worker->localWorkQueue.isEmpty()){
            try{
                worker->localWorkQueue.dequeue();
            }catch(const std::exception &ex){
                logger->error(std::format("Error while clearing worker local queue: {}", ex.what()));
            }
        }
    }
    workers.clear();
    logger->debug("ThreadPool stopped.");
}


threadPoolWorker *threadPool::getMinExpWorker(workerRole taskType){
    if(taskType == workerRole::NETWORK){ return workers[0].get(); }
    if(taskType == workerRole::DATABASE){ return workers[1].get(); }
    if(taskType == workerRole::LOG){ return workers[2].get(); }

    threadPoolWorker *minExpWorker = nullptr;
    size_t minQueueSize = threadPoolWorker::circularQueueCapacity + 1;

    for(size_t i = 3; i < workers.size(); i++){
        auto &worker = workers[i];
        if(worker->role == workerRole::COMMON){
            size_t localQueueSize = worker->localWorkQueue.getSize();
            if(localQueueSize < minQueueSize && !worker->localWorkQueue.isFull()){
                minQueueSize = localQueueSize;
                minExpWorker = worker.get();
            }
        }
    }
    return (minExpWorker != nullptr) ? minExpWorker : workers[3].get();
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
    stats.append("     THREADPOOL STATISTICS - UTILIZATION: ");
    stats.append(std::to_string(getThreadPoolPercentOccupancy()));
    stats.append("%            ");
    stats.append("\n");
    stats.append("--------------------------------------------------------");
    stats.append("\n");
    for(const auto &worker : workers){
        std::stringstream threadIdSs;
        threadIdSs << worker->thread_id;
        stats.append(std::format("Thread_ID: {}, Local Queue Size: {}\n", threadIdSs.str(), worker->localWorkQueue.getSize()));
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

void threadPool::setThreadCount(int threadCount){ this->threadCount = threadCount; }
