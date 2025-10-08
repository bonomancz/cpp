#include <iostream>
#include "../include/threadPool.hpp"
#include "../include/trackedFuture.hpp"


int main(int argc, char **argv){
    try{
        threadPool tp(8);
        tp.start();
        std::vector<trackedFuture<std::string>> results;
        while(tp.getThreadPoolRunningFlag()){
            // pushing tasks to queue
            std::cout << "Main loop cycle: ThreadPool size: " << tp.getThreadPoolSize() << std::endl;
            int taskCount = 10, start = 1;
            for(size_t i = start; i < (taskCount + start); i++){
                results.emplace_back(i, "Sample tread", std::chrono::steady_clock::now(), std::move(tp.threadPoolSubmitTask([](std::string inputString){ return inputString; }, "How are you doing?")));
            }
            
            // read futures results
            for(auto &result : results){
                std::cout << std::format("Id {}: Description: {}, Result: {}, Run time: {}usec", result.getId(), result.getDescription(), result.getTrackedFuture().get(), result.getElapsedTime<std::chrono::microseconds>()) << std::endl;
            }

            // remove empty futures
            for(auto it = results.begin(); it != results.end();){
                if(!it->getTrackedFuture().valid()){
                    results.erase(it);
                }else{
                    it++;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }catch(std::runtime_error &ex){
        std::cerr << ex.what() << std::endl;      
    }
}