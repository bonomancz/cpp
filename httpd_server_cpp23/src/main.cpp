#include <iostream>
#include "../include/httpdServer.hpp"

int main(int argc, char **argv){
    try{
        httpdServer server;
        server.processArguments(argc, argv);
        server.start();
    }catch(std::runtime_error &ex){
        std::cerr << ex.what() << std::endl;      
    }
}