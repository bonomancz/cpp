#include "utils.hpp"
#include "chronoTime.hpp"
#include "tariffSentenceData.hpp"


Utils::Utils(std::shared_ptr<ChronoTime> tmr, std::shared_ptr<tariffSentenceData> tdata) :
	timer(tmr),
	tData(tdata)
{}


std::string_view Utils::strip(const std::string_view inputString, std::string_view chars) {
	if (inputString.empty()) { return ""; }
	if (chars.empty()) { return inputString; }

	const auto begin = inputString.find_first_not_of(chars);
	if (begin == std::string::npos) { // cut characters only
		return {};
	}
		
	const auto end = inputString.find_last_not_of(chars);
	return inputString.substr(begin, end - begin + 1);
}


std::vector<std::string_view> Utils::split(std::string_view inputString, char delimiter) {
	std::vector<std::string_view> output;
	if (inputString.empty() || delimiter == '\0') { return output; }
	size_t first = 0;
	while(first < inputString.size()){
		const auto second = inputString.find(delimiter, first);
		if(second == std::string_view::npos){
			output.emplace_back(inputString.substr(first));
			break;
		}
		output.emplace_back(inputString.substr(first, second - first));
		first = second + 1;
	}
	
	return output;
}


tariffSentenceData Utils::parseTariffSentence(std::string_view inputString){
	tariffSentenceData tData;
	if(inputString.size() <= 60){
		throw std::runtime_error("Utils::parseTariffSentence(): Tariff sentence is too short.");
	}
	tData.dateTime = strip(inputString.substr(8, 2), " ");
	tData.dateTime += "-";
	tData.dateTime += strip(inputString.substr(6, 2), " ");
	tData.dateTime += "-";
	tData.dateTime += strip(inputString.substr(4, 2), " ");
	tData.dateTime += " ";
	tData.dateTime += strip(inputString.substr(11, 2), " ");
	tData.dateTime += ":";
	tData.dateTime += strip(inputString.substr(13, 2), " ");
	tData.dateTime += ":";
	tData.dateTime += timer->getCurrentTimeSeconds();
		
	tData.calling = strip(inputString.substr(18, 13), " ");	
	tData.dialled = strip(inputString.substr(38, 17), " ");
	tData.duration = stringViewToInt(strip(inputString.substr(61, 5), " "));
	auto trunkTmp = strip(inputString.substr(56, 4), " ");
	tData.trunk = trunkTmp.size() < 1 ? "in" : trunkTmp;
	return tData;
}


int Utils::stringViewToInt(std::string_view input){
	int output = 0;
	auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), output);
	if(ec == std::errc()){
		return output;
	}else if(ec == std::errc::invalid_argument){
		return -1;
	}else if(ec == std::errc::result_out_of_range){
		return -2;
	}else{
		return -3;	
	}
	return output;
}


bool Utils::isValidTariffSentence(const std::string_view sentence){
		return std::regex_match(std::string(sentence.data()), tariffSentencePattern);
	
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