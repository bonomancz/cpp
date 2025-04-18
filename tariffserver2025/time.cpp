#include "time.h"

std::string Time::getMillisecTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    localtime_r(&currentTime, &localTime);
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << millis.count();
    return oss.str();
}


std::string Time::getCurrentSecondsStr() {
    auto now = std::chrono::system_clock::now();
    auto todaysSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()) % 86400;
    long int seconds = todaysSeconds.count() % 60;
    std::string secondsStr = std::to_string(seconds);
    if (secondsStr.length() < 2) {
        secondsStr = "0" + secondsStr;
    }
    return secondsStr;
}
