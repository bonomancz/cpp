#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <openssl/err.h>
#include <openssl/provider.h>
#include <openssl/ssl.h>
#include "socketManager.hpp"


class LogManager;

class SslManager : public SocketManager {
private:
	SSL_CTX* sslCtx = nullptr;
	std::shared_ptr<LogManager> logger;
	std::string certificateFile = "./defaultBuildServerCert.pem";
	std::string privateKeyFile = "./defaultBuildPrivateKey.key";
public:
	explicit SslManager(std::shared_ptr<LogManager> lgMgr);
	~SslManager();
	void initializeContextSSL(const std::string &certificateFile, const std::string &privateKeyFile);
	SSL* acceptSsl(SOCKET clientSocket);
	void sendSockSSL(SSL* ssl, const std::string &data);
	void receiveSockSSL(SSL *ssl, std::string &rcvdMessage, const std::function<void(SSL *ssl, std::string&)> &onMessageReceived);
	void cleanupSockSSL(SSL* ssl);
	void setCertificateFile(const std::string &certFile);
	void setPrivateKeyFile(const std::string &keyFile);
};

