#include "databaseManager.hpp"
#include "logManager.hpp"
#include "tariffSentenceData.hpp"


databaseManager::databaseManager(std::shared_ptr<LogManager> lgMgr, std::shared_ptr<tariffSentenceData> tdata) : 
    logger{std::move(lgMgr)}, 
    tData{std::move(tdata)}
{}


databaseManager::~databaseManager(){
    try{
        stop();
    }catch(std::exception &ex){
        std::string exMsg = std::format("databaseManager::~databaseManager(): {}", ex.what());
        std::cerr << exMsg << std::endl;
    }
}


void databaseManager::initialize(){
    if(!mysql_init(&mysql)){
        if(mysql_error(&mysql)){
            std::string mysql_message = std::format("DatabaseManager::initialize(): {}", mysql_error(&mysql));
            throw std::runtime_error(mysql_message);       
        }
    }
    if(!connect()){
        std::string mysql_message = "DatabaseManager::initialize(): connect(): Database connection failed.";
        throw std::runtime_error(mysql_message);       
    }
    databaseRunningFlag = true;
    logger->debug("DatabaseManager initialized with active db connection.");
    isAvailable();
}


bool databaseManager::connect(){
    bool output{true};
    if(!mysql_real_connect(&mysql, this->host.c_str(), this->user.c_str(), this->password.c_str(), this->database.c_str(), this->port, NULL, 0)){
        std::string mysql_message = std::format("DatabaseManager::connect(): {}", mysql_error(&mysql));
        logger->debug(mysql_message);
        output = false;    
        throw std::runtime_error(mysql_message);       
    }
    return output;
}


bool databaseManager::disconnect(){
    if(!isAvailable()){
        databaseRunningFlag = false;
        return false;
    }
    mysql_close(&mysql);
    if(mysql_error(&mysql)){
        std::string mysql_message = std::format("DatabaseManager::disconnect(): {}", mysql_error(&mysql));
        throw std::runtime_error(mysql_message);
    }
    databaseRunningFlag = false;
    logger->debug("DatabaseManager disconnected.");
    return true;
}


void databaseManager::insert(std::string_view mysqlQuery){
    if(mysql_real_query(&mysql, mysqlQuery.data(), mysqlQuery.size()) != 0){
        std::string mysql_message = std::format("DatabaseManager::insert(): error: {}", mysql_error(&mysql));
        logger->error(mysql_message);
    }
}


void databaseManager::start(){
    logger->debug("DatabaseManager starting.");
    initialize();
}


void databaseManager::stop(){
    try{
        logger->debug("DatabaseManager stopping.");
        if(!databaseRunningFlag){ return; }
        disconnect();
        databaseRunningFlag = false;
        logger->debug("DatabaseManager stopped.");
    }catch(std::exception &ex){
        std::string exMsg = std::format("databaseManager::stop(): {}", ex.what());
        std::cerr << exMsg << std::endl;
    }
}


bool databaseManager::isAvailable(){
    bool output{false};
    const char* mysqlQuery = "SELECT 1;";
    std::string mysql_message;    
    if(mysql_query(&mysql, mysqlQuery) == 0){
        databaseRunningFlag = true;
        output = true;
        mysql_message = "MySQL database is available for queries.";    
        MYSQL_RES *result = mysql_store_result(&mysql);
        if(result){ mysql_free_result(result); }    
    }else{
        databaseRunningFlag = false;
        output = false;
        mysql_message = std::format("DatabaseManager::isAvailable(): Error. MySQL database is not available for queries: {}", mysql_error(&mysql));
    }
    logger->debug(mysql_message);    
    return output;
}


void databaseManager::enqueueTariffData(tariffSentenceData tData){
    logger->debug("pushing tariff data to buffer: " + tData.dateTime + " " + tData.calling + " " + tData.dialled + " " + std::to_string(tData.duration) + " " + tData.trunk);
    tariffDataBuffer.push(std::move(tData));
    if(tariffDataBuffer.size() >= 100){
        flushTariffDataBufferToSql();
    }
}


void databaseManager::flushTariffDataBufferToSql(){
    if(tariffDataBuffer.empty()){ return; }
    std::string mysqlQuery = std::format("insert into tarifikace.definity (`datetime`, `calling`, `dialled`, `duration`, `code`) values ");
    std::string tempSqlStr{""}, delimiter{""};
    while(!tariffDataBuffer.empty()){
        auto tData = std::move(tariffDataBuffer.front());
        tariffDataBuffer.pop();
        tempSqlStr = std::format("{}{}('{}', '{}', '{}', {}, '{}')", tempSqlStr, delimiter, tData.dateTime, tData.calling, tData.dialled, tData.duration, tData.trunk);
        delimiter = ", ";
    }
    mysqlQuery = std::format("{}{};", mysqlQuery, tempSqlStr);
    // logger->trace(mysqlQuery);
    this->insert(mysqlQuery);
}


void databaseManager::setDatabaseHost(const std::string &server){ host = server; }
void databaseManager::setDatabaseUser(const std::string &user){ this->user = user; }
void databaseManager::setDatabasePassword(const std::string &password){ this->password = password; }
void databaseManager::setDatabaseName(const std::string &database){ this->database = database; }
void databaseManager::setDatabasePort(const int port){ this->port = port; }
