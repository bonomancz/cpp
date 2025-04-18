#pragma once
#include <iostream>
#include <string>
#include <mariadb/mysql.h>
#include "Interface_DataStorage.h"

class Config;
class MysqlDb : public Interface_DataStorage {
private:
	MYSQL *connection = nullptr, mysql;
	MYSQL_RES* result;
	MYSQL_ROW row;
	MYSQL_FIELD* field;
	std::string mysqDatabase, mysqlTable;
	bool initialized = false;
	std::string buildQuery(TariffSentence& sentence);
	bool execQuery(std::string& query);

public:
	MysqlDb();
	~MysqlDb();
	bool connect(Config& conf) override;
	bool isDataStorageInitialized(Config &conf) override;
	bool pushToStorage(TariffSentence& sentence) override;
	bool disconnect() override;
};

