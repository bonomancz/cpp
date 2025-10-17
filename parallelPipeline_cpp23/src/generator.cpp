#include "../include/generator.hpp"


generator::generator(){}


generator::~generator(){
    stop();
}


int generator::generatePseudoRand(){
    try{
        auto seed = static_cast<unsigned>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count() ^
            std::hash<std::thread::id>{}(std::this_thread::get_id()) ^
            std::random_device{}()
        );
        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(1,1000);
        return dist(gen);
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
        return 0;
    }
}


void generator::start(){
    try{
        generatorWorker = std::jthread([this](std::stop_token st){
            while(!st.stop_requested() && generatorRunningFlag){
                {
                    pushNewTasks(500);
                    if(st.stop_requested() || !generatorRunningFlag){ break; }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            }
            std::cout << "Generator thread ended. " << std::endl;
        });
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }
}


void generator::stop(){
    try{
        generatorRunningFlag = false;
        generatorWorker.request_stop();
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }
}


void generator::pushNewTasks(int count){
    try{
        if(generatorQueue != nullptr){
            for(size_t i = 0; i < count; i++){
                generatorQueue->emplace(generatePseudoRand());
            }
        }
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }
}


std::shared_ptr<threadSafeQueue<int>> generator::getGeneratorQueue() const{ return generatorQueue; }