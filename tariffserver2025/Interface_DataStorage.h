#pragma once
#include <string>

class Config;
class TariffSentence;

class Interface_DataStorage {
public:
	virtual ~Interface_DataStorage() = default;
	virtual bool isDataStorageInitialized(Config& conf) = 0;
	virtual bool connect(Config& conf) = 0;
	virtual bool pushToStorage(TariffSentence& sentence) = 0;
	virtual bool disconnect() = 0;
};