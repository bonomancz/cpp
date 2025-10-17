#include "../include/parallelPipeline.hpp"


parallelPipeline::parallelPipeline(){};


parallelPipeline::~parallelPipeline(){ stop(); }


void parallelPipeline::start(){
    try{
        gnr.start();
        wrk.setGeneratorQueue(gnr.getGeneratorQueue());
        wrk.setConsumerQueue(csm.getConsumerQueue());
        wrk.start();
        csm.start();
        std::cout << "Parallel pipeline started." << std::endl;
        while(pipelineRunningFlag){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "Parallel pipeline ended." << std::endl;
    }catch(std::runtime_error &ex){
        std::cerr << "parallelPipeline::start(): " << ex.what() << std::endl;
    }
}


void parallelPipeline::stop(){
    gnr.stop();
    wrk.stop();
    csm.stop();
    if(gnr.getGeneratorQueue()){ gnr.getGeneratorQueue()->setDone(); }
    if(csm.getConsumerQueue()){ csm.getConsumerQueue()->setDone(); }
    pipelineRunningFlag = false;
}


bool parallelPipeline::getPipelineRunningFlag(){ return pipelineRunningFlag; }
