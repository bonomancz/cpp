#include "chronoTime.hpp"


ChronoTime::ChronoTime(){}


ChronoTime::~ChronoTime(){
    stop();
}


void ChronoTime::start(){
    try{
        std::cout << "ChronoTimer thread ended. " << std::endl;
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }
}


void ChronoTime::stop(){
    try{
    }catch(std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }
}


std::string ChronoTime::getCurrentTimeMs() const{
        try{
            auto current = std::chrono::system_clock::now();
            auto seconds = floor<std::chrono::seconds>(current);
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(current - seconds).count();
            std::chrono::zoned_time localTime{tzPrague, seconds};            
            return std::format("{:%Y-%m-%d %H:%M:%S}.{:03d}", localTime, static_cast<int>(milliseconds));
        }catch(std::exception &ex){
            std::cerr << "getCurrentTime(): " << ex.what() << std::endl;
            return{};
        }
}


int64_t ChronoTime::getTimeStamp() const{
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}


std::string ChronoTime::getTimeFromTimeStamp(int64_t timeStamp) const{
    using namespace std::chrono;
    auto timePoint = floor<seconds>(system_clock::time_point{seconds{timeStamp}});
    zoned_time localTime{tzPrague, timePoint};
    return std::format("{:%Y-%m-%d %H:%M:%S}", localTime);
}
