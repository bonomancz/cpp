#include "../include/worker.hpp"


worker::worker(){}


worker::~worker(){
    stop();
}


void worker::setGeneratorQueue(std::shared_ptr<threadSafeQueue<int>> generatorQueuePtr){
    generatorQueue = generatorQueuePtr;
}


void worker::setConsumerQueue(std::shared_ptr<threadSafeQueue<int>> consumerQueuePtr){
    consumerQueue = consumerQueuePtr;
}


int worker::getGeneratorQueueSize(){ return generatorQueue->size(); }


int worker::getConsumerQueueSize(){ return consumerQueue->size(); }


int worker::workerTask(int inputNo){ return inputNo * 1000; }


void worker::start(){
    try{
        workerThread = std::jthread([this](std::stop_token st){
            int generatorQueueItem;
            while(!st.stop_requested() && workerRunningFlag){
                if(generatorQueue->pop(generatorQueueItem)){
                    consumerQueue->emplace(workerTask(generatorQueueItem));
                }
            }
            std::cout << "Worker thread ended. " << std::endl;
        });
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }
}


void worker::stop(){
    try{
        workerRunningFlag = false;
        workerThread.request_stop();
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }
}