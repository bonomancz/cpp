#include <iostream>
#include "../include/parallelPipeline.hpp"

int main(int argc, char **argv){
    try{
        parallelPipeline pp;
        pp.start();
    }catch(std::runtime_error &ex){
        std::cerr << ex.what() << std::endl;      
    }
}