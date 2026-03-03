#include "threadPoolMain.hpp"


threadPoolMain::threadPoolMain() : 
    utils{}, 
    timer{},
    threadCount{4}, 
    threads{threadCount, logger}, 
    refTimer{timer}, 
    logger{refTimer}, 
    refLogger{logger},
    taskWeight{37}
{}


threadPoolMain::~threadPoolMain(){ this->stop(); }


void threadPoolMain::runMainServerLoop(std::stop_token stopToken){
    threads.start();

    // register stop callback
    std::stop_callback stopCallback(stopToken, [this](){
        logger.debug("Stop callback triggered. Stopping server main loop.");
        mainLoopCv.notify_one();
    });


    this->startMonitoringThread();
    this->sendTestTasksToThreadPool();

    
    // Wait for stop signal
    {
        std::unique_lock<std::mutex> lock(mainLoopMtx);
        mainLoopCv.wait(lock, [&stopToken](){ return stopToken.stop_requested(); });
    }
}


void threadPoolMain::startMonitoringThread(){
    int monitoringIntervalMs = 400;
    monitoringThread = std::jthread([this, monitoringIntervalMs](std::stop_token sttk){
        while(!sttk.stop_requested()){
            // logger.debug(std::format("Active threads: {}, Thread pool size: {}, Current RingBuffer size: {}", threads.getActiveThreadCount(), threads.getThreadPoolSize(), threads.getCircularQueueSize()));
            logger.debug(threads.getThreadPoolQueuesStatistics());
            std::this_thread::sleep_for(std::chrono::milliseconds(monitoringIntervalMs));
        }
    });
}


void threadPoolMain::sendTestTasksToThreadPool(){
    int taskSubmitIntervalMs = 2;
    int taskSubmitDelayMs = 800;
    tasksThread = std::jthread([this, taskSubmitIntervalMs, taskSubmitDelayMs](std::stop_token sttk){
        while(!sttk.stop_requested()){
            if(!threads.getThreadPoolOccupancy(1.0)){
                logger.warning("Thread pool capacity limit reached. Waiting.");
                std::this_thread::sleep_for(std::chrono::milliseconds(taskSubmitDelayMs));
                continue;
            }
            threads.threadPoolSubmitTask([this](){ this->heavyTask(); });
            std::this_thread::sleep_for(std::chrono::milliseconds(taskSubmitIntervalMs));
        }
    });
}


void threadPoolMain::timedOutShutdown(int secondsTimeout){
    auto timedOutShutdownFuture = std::async(std::launch::async, [this, secondsTimeout](){
        std::this_thread::sleep_for(std::chrono::seconds(secondsTimeout));
        logger.debug("Timed out shutdown triggered. Stopping server.");
        mainLoopStopSource.request_stop();
    });
}


void threadPoolMain::start(){ this->runMainServerLoop(mainLoopStopSource.get_token()); }


void threadPoolMain::stop(){
    tasksThread.request_stop();
    threads.stop();
    monitoringThread.request_stop();
    logger.stop();
}

void threadPoolMain::shutdown(){ mainLoopStopSource.request_stop(); }


int64_t threadPoolMain::fibTask(int64_t n) { return n < 2 ? n : fibTask(n - 1) + fibTask(n - 2); }


void threadPoolMain::heavyTask(){
    std::string durationStr;
    auto startMeasurement = std::chrono::high_resolution_clock::now();
    int64_t fibResult = this->fibTask(taskWeight);
    durationStr = refTimer.getTimeDuration<std::chrono::high_resolution_clock, std::chrono::microseconds>(startMeasurement);
    // logger.debug(std::format("Test task result: {}, duration: {} [sec], thread ID: {}", fibResult, durationStr, Utils::threadIdToString(std::this_thread::get_id())));
}
