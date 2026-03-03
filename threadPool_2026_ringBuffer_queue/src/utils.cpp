#include "utils.hpp"


std::string Utils::strip(const std::string& inputString, const std::string &chars) {
	try {
		size_t begin = inputString.find_first_not_of(chars);
		if (begin == std::string::npos) {
			return "";
		}
		size_t end = inputString.find_last_not_of(chars);
		return inputString.substr(begin, end - begin + 1);
	}
	catch (std::runtime_error &ex) {
		std::cerr << ex.what() << std::endl;
		return {};
	}
}


std::vector<std::string> Utils::split(const std::string &inputString, char delimiter) {
	std::vector<std::string> output;
	std::stringstream ss(inputString);
	std::string token;
	while (getline(ss, token, delimiter)) {
		output.emplace_back(token);
	}
	return output;
}


bool Utils::isNumeric(const std::string &input) {
	return !input.empty() && std::all_of(input.begin(), input.end(), ::isdigit);
}


std::string Utils::threadIdToString(std::thread::id threadId) {
	std::ostringstream oss;
	oss << threadId;
	return oss.str();
}


bool Utils::isInteger(const std::string &s) {
    if (s.empty()) return false;
    return std::all_of(s.begin(), s.end(), ::isdigit);
}


bool Utils::isFloat(const std::string &s) {
    char* end = nullptr;
    std::strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0';
}


bool Utils::isBoolean(const std::string &s) {
    return s == "true" || s == "false" || s == "1" || s == "0";
}


bool Utils::isString(const std::string &s) {
    return !isInteger(s) && !isFloat(s) && !isBoolean(s);
}


bool Utils::isPrime(const unsigned long long n){
	if (n <= 1) { return false; } // 0 a 1 nejsou prvočísla
	if (n <= 3) { return true; }  // 2 a 3 jsou prvočísla
	if (n % 2 == 0 || n % 3 == 0) { return false; } // Vyloučíme násobky 2 a 3

	// Všechna prvočísla větší než 3 lze vyjádřit ve tvaru 6k ± 1
	for (unsigned long long i = 5; i * i <= n; i += 6) {
		if (n % i == 0 || n % (i + 2) == 0){
			return false;
		}
	}
	return true;
}


int Utils::getRandomNumber(int from, int to){
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(from, to);
	return dist(rng);
}


bool Utils::isEven(unsigned long long n){ return n % 2 == 0; }
bool Utils::isOdd(unsigned long long n){ return !isEven(n); }