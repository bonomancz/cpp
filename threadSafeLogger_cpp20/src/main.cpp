#include <iostream>
#include "../include/threadSafeLogger_cpp20.hpp"

int main(int argc, char **argv){
    try{
        ThreadSafeLogger tsl;
        tsl.start();
        while(true && !tsl.getServiceStopFlag()){
            tsl.log(1, "2025-10-05 14:41:00.951 [DEBUG] System hearbeat, system running ok.");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }catch(std::runtime_error &ex){
        std::cerr << ex.what() << std::endl;      
    }
}