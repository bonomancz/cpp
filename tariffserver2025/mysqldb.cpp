#include "mysqldb.h"
#include "config.h"


MysqlDb::MysqlDb() {
	mysql_init(&mysql);
}


MysqlDb::~MysqlDb() {
	this->disconnect();
}


bool MysqlDb::isDataStorageInitialized(Config &conf) {
	if (!this->initialized) {
		this->connect(conf);
	}
	return this->initialized;
}


bool MysqlDb::connect(Config &conf) {
	bool result = false;
	try {
		unsigned int mysqlPort = conf.get_mysqlPort();
		if ((connection = mysql_real_connect(&mysql, conf.get_mysqlServer().c_str(), conf.get_mysqlUser().c_str(), conf.get_mysqlPassword().c_str(), conf.get_mysqlDatabase().c_str(), mysqlPort, nullptr, 0))) {
			this->mysqDatabase = conf.get_mysqlDatabase();
			this->mysqlTable = conf.get_mysqlTable();
			result = true;
			this->initialized = true;
		}
		else {
			result = false;
			unsigned int mysqlErrNo = mysql_errno(&mysql);
			std::string mysqlErrMsg = std::string(mysql_error(&mysql));
			throw std::runtime_error("Failed to connect to MySQL DB server. Mysql error: " + std::to_string(mysqlErrNo) + " (" + mysqlErrMsg + ")");
		}
	}
	catch (std::runtime_error &ex) {
		throw std::runtime_error("MysqlDb::connect(): " + std::string(ex.what()));
	}
	return result;
}


std::string MysqlDb::buildQuery(TariffSentence& sentence) {
	try {
		return "insert into " + this->mysqDatabase + "." + this->mysqlTable + " (`datetime`, `calling`, `dialled`, `code`, `duration`) values ('" + sentence.dbDateTime + "', '" + sentence.ani + "', '" + sentence.dnis + "', '" + sentence.trunk + "', '" + std::to_string(sentence.duration) + "');";
	}
	catch (std::runtime_error& ex) {
		throw std::runtime_error("MysqlDb::buildQuery(): " + std::string(ex.what()));
	}
}


bool MysqlDb::execQuery(std::string &query) {
	bool retVal = true;
	try {
		unsigned int mysqlErrNo = mysql_errno(&mysql);
		std::string mysqlErrMsg = std::string(mysql_error(&mysql));
		if (mysql_query(connection, query.c_str()) != 0) {
			retVal = false;
			throw std::runtime_error("Failed to exec MySQL query. Mysql error: " + std::to_string(mysqlErrNo) + " (" + mysqlErrMsg + ")");
		}
	}
	catch (std::runtime_error& ex) {
		throw std::runtime_error("MysqlDb::execQuery(): " + std::string(ex.what()));
	}
	return retVal;
}


bool MysqlDb::pushToStorage(TariffSentence& sentence) {
	try {
		std::string query = buildQuery(sentence);
		execQuery(query);
		return true;
	}
	catch (std::runtime_error& ex) {
		throw std::runtime_error("MysqlDb::pushToStorage(): " + std::string(ex.what()));
	}
}


bool MysqlDb::disconnect() {
	mysql_close(connection);
	return true;
}
