#pragma once
#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>

class Time{
private:
public:
	std::string getMillisecTime();
	std::string getTimeFromTimestamp(long int timeStamp);
	unsigned int getTimeStamp();
};

