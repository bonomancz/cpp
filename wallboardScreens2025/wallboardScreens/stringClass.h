#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class StrManip{
public:
	std::string charToStr(char* chr);											// converts char to string
	int getLen(std::string str);												// gets string length
	std::vector<std::string> getToks(std::string str);							// builds tokens array
	void printToks(std::vector<std::string> strArr);
	std::string removeWhitespace(std::string st);
	std::string strip(const std::string& inputString, std::string chars);
	std::vector<std::string> split(const std::string& inputString, char delimiter);
};
