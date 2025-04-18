#include "data.h"


bool Data::pushToStorage(TariffSentence& sentence) {
	return mysqldb.pushToStorage(sentence);
}


bool Data::isDataStorageInitialized(Config &conf) {
	return mysqldb.isDataStorageInitialized(conf);
}


void Data::processBufferToStorage() {
	while (true) {
		std::unique_lock<std::mutex> lock(queueBufferMutex);
		queueCv.wait(lock, [&] {
			return !queueBuffer.empty();
		});

		while (!queueBuffer.empty()) {
			TariffSentence sentence = queueBuffer.front();
			queueBuffer.pop_front();
			lock.unlock();
			pushToStorage(sentence);
			lock.lock();
		}
	}
}


void Data::insertDataToBuffer(TariffSentence tariffData) {
	{
		std::lock_guard<std::mutex> lock(queueBufferMutex);
		this->queueBuffer.push_back(tariffData);
	}
	this->queueCv.notify_one();
}


void Data::processRcvdMessage(const std::string message) {
	try {
		std::istringstream messageStream(message);
		std::string line;
		while (std::getline(messageStream, line)) {
			if (auto sentenceData = parseTariffSentence(line)){
				insertDataToBuffer(*sentenceData);
			}
		}
	}
	catch (std::exception &ex) {
		throw std::runtime_error("processRcvdMessage(): Exception: " + std::string(ex.what()));
	}
}


const std::regex Data::tariffSentencePattern(R"(^\s*(\d)\s+(\d{6})\s+(\d{4})\s+([0-9#]+)\s+([0-9#]+)\s+(?:(\d{4})\s+)?(\d{5})\s*$)");


bool Data::isValidTariffSentence(const std::string& sentence) {
	return regex_match(sentence, tariffSentencePattern);
}


std::optional<TariffSentence> Data::parseTariffSentence(const std::string& sentence) {
	TariffSentence parsedData;
	try {
		std::smatch match;
		if (!std::regex_match(sentence, match, tariffSentencePattern)) {
			return std::nullopt;
		}
		parsedData.type = std::stoi(match[1]);
		parsedData.date = match[2];
		parsedData.time = match[3];
		std::string rawDate = match[2];
		std::string rawDay = rawDate.substr(0, 2);
		std::string rawMonth = rawDate.substr(2, 2);
		std::string rawYear = rawDate.substr(4, 2);
		rawDate = rawYear + "-" + rawMonth + "-" + rawDay;
		std::string rawTime = match[3];
		std::string rawHours = rawTime.substr(0,2);
		std::string rawMinutes = rawTime.substr(2, 2);
		std::string rawSeconds = tm.getCurrentSecondsStr();
		rawTime = rawHours + ":" + rawMinutes + ":" + rawSeconds;
		parsedData.dbDateTime = rawDate + " " + rawTime;
		parsedData.dnis = match[4];
		parsedData.ani = match[5];
		if (match[6].length() < 1) {
			parsedData.trunk = "in";
		}
		else {
			parsedData.trunk = match[6];
		}
		parsedData.duration = std::stoi(match[7]);
	}
	catch (std::exception& ex) {
		throw std::runtime_error("parseTariffSentence(): " + std::string(ex.what()));
	}
	return parsedData;
}
