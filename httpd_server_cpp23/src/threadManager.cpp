#include "../include/threadManager.hpp"
#include "../include/logManager.hpp"


ThreadManager::ThreadManager(std::shared_ptr<LogManager> lgMgr) : logger(lgMgr){}


ThreadManager::~ThreadManager(){ stop(); }


void ThreadManager::stop(){
    try{
        threadManagerRunningFlag = false;
        threadPoolTaskQueue.setDone();
        logger->debug("ThreadManager: STOPPING. All thread notified.");
        for(int i = 0; i < threadCount; i++){
            threadPool[i].request_stop();
            logger->debug(std::format("ThreadManager: Thread[{}] stop requested.", i));
        }
    }catch(std::exception &ex){
        std::string exMsg = std::format("ThreadManager::stop(): {}", ex.what());
        std::cerr << exMsg << std::endl;
        logger->debug(exMsg);
    }
}


void ThreadManager::start(){
    for(int i = 0; i < threadCount; i++){
        threadPool.emplace_back([&, i](std::stop_token stt){
            while(!stt.stop_requested() && threadManagerRunningFlag){
                std::function<void()> task;
                if(!threadPoolTaskQueue.pop(threadManagerRunningFlag, task)){ return; }
                logger->trace(std::format("ThreadManager: Thread[{}-(thread id: {})] pop from task queue.", i, std::this_thread::get_id()));
                try{
                    task();
                    logger->trace(std::format("ThreadManager: Thread[{}-(thread id: {})] executed task from task queue.", i, std::this_thread::get_id()));
                }catch(const std::exception &ex){
                    std::string exMsg = std::format("ThreadManager::start(): Running Thread[{}-(thread id: {})]: {}", i, std::this_thread::get_id(), ex.what());
                    std::cerr << exMsg << std::endl;
                    logger->error(exMsg);
                }catch(...){
                    std::string exMsg = std::format("ThreadManager::start(): Running Thread[{}-(thread id: {})]: Unknown exception.", i, std::this_thread::get_id());
                    std::cerr << exMsg << std::endl;
                    logger->error(exMsg);
                }
            }
        });
    }
}


int ThreadManager::getTaskQueueSize() const{ return threadPoolTaskQueue.size(); }
