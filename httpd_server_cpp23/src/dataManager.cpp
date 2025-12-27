#include "../include/dataManager.hpp"


DataManager::DataManager(){}


DataManager::~DataManager(){}


std::string DataManager::getHttpReponse(int httpResultCode) {
	std::string textColor, responseText;
	
	switch (httpResultCode) {
		case 200:	textColor = "green";
					responseText = "HTTP/1.1 200 OK (Server: " + serverHostName + ")";
					break;
		case 404:	textColor = "red";
					responseText = "HTTP/1.1 404 Not Found  (Server: " + serverHostName + ")";
					break;
		case 500:	textColor = "red";
					responseText = "HTTP/1.1 500 Internal Server Error  (Server: " + serverHostName + ")";
					break;
		default:	textColor = "green";
					responseText = "HTTP/1.1 200 OK (Server: " + serverHostName + ")";
					break;
	}

	std::string content =
		"<html>"
		"<head>"
		"<title>HTTP Server " + serverHostName + "</title>"
		"</head>"
		"<body style=\"background-color: white;\">"
		"<h1 style=\"color: " + textColor + "; font-family: Tahoma; font-size: 24px;\">" + responseText + "</h1>"
		"</body>"
		"</html>";

	std::string response = "" + responseText + "\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Content-Length: " + std::to_string(content.length()) + "\r\n"
		"Server: HTTP Server" + "\r\n"
		"Connection: close\r\n"
		"Date: Mon, 02 Sep 2024 12:00:00 GMT\r\n"
		"\r\n";
	response += content;
	return response;
}

bool DataManager::isHttpRequest(std::string &message) {
	bool output = false;
	std::regex httpRequestPattern(R"((GET|POST|PUT|DELETE|HEAD|CONNECT|OPTIONS|TRACE|PATCH)\s+\/[^\s]*\s+HTTP\/1\.1)");
	std::regex headerEndPattern(R"(\r\n\r\n)");
	std::regex userAgentPattern(R"(User-Agent:\s*.+)");
	if (std::regex_search(message, httpRequestPattern) && std::regex_search(message, headerEndPattern) && std::regex_search(message, userAgentPattern)) {
		output = true;
	}
	return output;
}

bool DataManager::isExistingUrl(std::string& message) {
	bool output = false;
	std::string requestUrl = this->getRequestUrl(message);
	std::list<std::string> validUrls = {
		"/",
		"/test",
		"/json",
		"/request",
		"/data"
	};
	for (const auto& url : validUrls) {
		if (url == requestUrl) {
			output = true;
		}
	}
	return output;
}


std::string DataManager::getRequestUrl(std::string& message) {
	std::string output = "";
	size_t start = message.find(" ") + 1;
	size_t end = message.find(" ", start);
	if (start != std::string::npos && end != std::string::npos) {
		output = message.substr(start, end - start);
	}
	return output;
}


void DataManager::setServerHostName(const std::string &hostName){ serverHostName = hostName; }


std::string DataManager::getClientRequest(std::string& message) {
	std::string output;
	std::istringstream stream(message);
	std::string line, methodPath, userAgent;
	if (getline(stream, line)) {
		std::istringstream firstLine(line);
		std::string method, path;
		firstLine >> method >> path;
		methodPath = method + " " + path;
	}

	while (getline(stream, line)) {
		if (line.rfind("User-Agent:", 0) == 0) {
			userAgent = line.substr(12);
			break;
		}
	}
	output = methodPath + ", " + userAgent;
	return output;
}

std::string DataManager::strip(const std::string& inputString, std::string chars) {
	try {
		size_t begin = inputString.find_first_not_of(chars);
		if (begin == std::string::npos) {
			return "";
		}
		size_t end = inputString.find_last_not_of(chars);
		return inputString.substr(begin, end - begin + 1);
	}
	catch (std::runtime_error rex) {
		std::cerr << rex.what() << std::endl;
		return "";
	}
}

std::vector<std::string> DataManager::split(const std::string& inputString, char delimiter) {
	std::vector<std::string> output;
	std::stringstream ss(inputString);
	std::string token;
	while (getline(ss, token, delimiter)) {
		output.emplace_back(token);
	}
	return output;
}