#include "../include/file.hpp"


File::File(){}


std::string File::read(const std::string &fileName){
    try {
        std::ifstream file(fileName);
        file.exceptions(std::ios::failbit | std::ios::badbit);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    catch (const std::ios_base::failure &fex) {
        std::cerr << "File::read(): " << fex.what() << std::endl;
        return {};
    }
}


void File::write(const std::string &fileName, const std::string &data){
    try{
        std::fstream file(fileName, std::ios::out | std::ios::app);
        file.exceptions(std::ios::failbit | std::ios::badbit);
        file << data << std::endl;
        file.flush();
    }catch(const std::ios_base::failure &fex){
        std::cerr << "File::write(): " << std::string(fex.what()) << std::endl;
    }
}