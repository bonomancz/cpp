#pragma once
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include <string_view>


class ChronoTime{
private:
    std::jthread chronoTimerWorker;
    std::atomic<bool> chronoTimerRunningFlag{true};
    const std::chrono::time_zone *tzPrague = std::chrono::locate_zone("Europe/Prague");
public:
    ChronoTime();
    ~ChronoTime();
    void start();
    void stop();
    int64_t getTimeStamp() const;
    std::string getTimeFromTimeStamp(int64_t timeStamp) const;
    std::string getCurrentTimeMs() const;
    std::string getCurrentTimeSeconds() const;

    template<typename Clock = std::chrono::high_resolution_clock, typename Duration = std::chrono::microseconds>
    std::string getTimeDuration(const std::chrono::time_point<Clock> &startTimePoint) const noexcept{
        try{
            std::string outputStr;
            auto timeRslt = Clock::now() - startTimePoint;
            auto secondsRslt = std::chrono::duration_cast<std::chrono::seconds>(timeRslt);
            int seconds = secondsRslt.count();
            if constexpr (std::is_same_v<Duration, std::chrono::microseconds>){
                auto microsecondsRslt = std::chrono::duration_cast<Duration>(timeRslt - secondsRslt);
                int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(microsecondsRslt).count();
                int microseconds = microsecondsRslt.count() % 1000;
                outputStr = std::format("{}.{:03d},{:03d}", seconds, milliseconds, microseconds);
            }else if constexpr (std::is_same_v<Duration, std::chrono::milliseconds>){
                auto millisecondsRslt = std::chrono::duration_cast<Duration>(timeRslt - secondsRslt);
                int milliseconds = millisecondsRslt.count();
                outputStr = std::format("{}.{:03d}", seconds, milliseconds);
            }
            return outputStr;
        }catch(std::exception &ex){
            std::cerr << "getTimeDuration(): " << ex.what() << std::endl;
            return {};
        }
    }
};