#include "../include/consumer.hpp"


consumer::consumer(){}


consumer::~consumer(){ stop(); }


void consumer::start(){
    try{
        consumerWorker = std::jthread([this](std::stop_token st){
            int consumerQueueItem;
            while(!st.stop_requested() && consumerRunningFlag){
                if(st.stop_requested() || !consumerRunningFlag){ break; }
                if(consumerQueue->pop(consumerQueueItem)){ std::cout << "Consumer result: " << std::to_string(consumerQueueItem) << std::endl; }
            }
            std::cout << "Consumer thread ended." << std::endl;
        });
    }catch(std::exception &ex){
        std::cout << "consumer::start(): " << ex.what() << std::endl;
    }
}


void consumer::stop(){
    try{
        consumerRunningFlag = false;
        consumerWorker.request_stop();
    }catch(std::exception &ex){
        std::cout << "consumer::stop(): " << ex.what() << std::endl;
    }
}


std::shared_ptr<threadSafeQueue<int>> consumer::getConsumerQueue() const { return consumerQueue; }