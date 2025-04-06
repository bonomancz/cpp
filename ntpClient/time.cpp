#include "time.h"

std::string Time::getMillisecTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    localtime_s(&localTime, &currentTime);
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << millis.count();
    return oss.str();
}

unsigned int Time::getTimeStamp() {
    unsigned int timeStamp = 0;
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    timeStamp = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>(epoch).count());
    return timeStamp;
}

std::string Time::getTimeFromTimestamp(long int timeStamp) {
    time_t rawTime = timeStamp;
    struct tm myTime;
    char buffer[30];
    localtime_s(&myTime, &rawTime);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", &myTime);
    return std::string(buffer);
}