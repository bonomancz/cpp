#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <queue>
#include <memory>
#include <atomic>
#include <mariadb/mysql.h>
#include "iDataStorage.hpp"


class LogManager;
struct tariffSentenceData;


class databaseManager : public iDataStorage{
private:
    MYSQL *connection = nullptr, mysql;
    MYSQL_RES *result = nullptr;
    MYSQL_ROW row;
    std::string database, host, user, password;
    int port;
    std::shared_ptr<LogManager> logger;
    std::shared_ptr<tariffSentenceData> tData;
    std::atomic<bool> databaseRunningFlag{false};
    std::queue<tariffSentenceData> tariffDataBuffer;    

public:
    databaseManager(std::shared_ptr<LogManager> lgMgr, std::shared_ptr<tariffSentenceData> tdata);
    ~databaseManager();
    void initialize() override;
    bool isAvailable() override;
    bool connect() override;
    bool disconnect() override;
    void insert(std::string_view mysqlQuery) override;
    void start() override;
    void stop() override;
    void enqueueTariffData(tariffSentenceData tData);
    void flushTariffDataBufferToSql();
    
    void setDatabaseHost(const std::string &server);
    void setDatabaseUser(const std::string &user);
    void setDatabasePassword(const std::string &password);
    void setDatabaseName(const std::string &database);
    void setDatabasePort(const int port);
};