#include "../include/sslManager.hpp"
#include "../include/logManager.hpp"


SslManager::SslManager(std::shared_ptr<LogManager> lgMgr) : SocketManager(lgMgr), logger(lgMgr){
    SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}


SslManager::~SslManager(){
    if(sslCtx){
        SSL_CTX_free(sslCtx);
    }
}


void SslManager::initializeContextSSL(const std::string &certFile, const std::string &keyFile) {
    if (!(sslCtx = SSL_CTX_new(TLS_server_method()))) {
		throw std::runtime_error("initializeContextSSL(): Failed to create SSL context.");
	}
    SSL_CTX_set_min_proto_version(sslCtx, TLS1_2_VERSION);
    SSL_CTX_set_options(sslCtx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
    SSL_CTX_set_ecdh_auto(sslCtx, 1);
    if (SSL_CTX_use_certificate_file(sslCtx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(sslCtx, keyFile.c_str(), SSL_FILETYPE_PEM) <= 0 ||
        !SSL_CTX_check_private_key(sslCtx)){
        throw std::runtime_error("SslManager::initializeSslContext(): SSL certificate/key error.");
    }
    logger->info("SslManager: SSL context initialized.");
}


void SslManager::sendSockSSL(SSL* ssl, const std::string &data) {
	if (SSL_write(ssl, data.c_str(), static_cast<int>(data.size())) < 1) {
		throw std::runtime_error("SSL data send failed.");
	}
}


SSL* SslManager::acceptSsl(SOCKET clientSocket) {
    SSL* ssl = SSL_new(sslCtx);
    if(!ssl){
        throw std::runtime_error("SslManager::acceptSsl(): SSL_new() failed.");
    }
    SSL_set_fd(ssl, static_cast<int>(clientSocket));
    int rslt = SSL_accept(ssl);
    if (rslt <= 0) {
        int sslErr = SSL_get_error(ssl, rslt);
        unsigned long errCode = ERR_get_error();
        std::string errStr = ERR_error_string(errCode, nullptr);
        std::string exMsg = std::format("SslManager::acceptSsl(): SSL_accept failed: {}, err={}", sslErr, errStr);
        logger->error(exMsg);
        SSL_free(ssl);
        throw std::runtime_error(exMsg);
    }
    logger->trace("SslManager::acceptSsl(): SSL handshake successful.");
    return ssl;
}


void SslManager::cleanupSockSSL(SSL* ssl) {
    if(!ssl){ return; }
	int rslt = SSL_shutdown(ssl);
    if(rslt == 0){ SSL_shutdown(ssl); }
    SSL_free(ssl);
	logger->trace("SslManager::cleanupSockSSL(): SSL connection cleaned up.");
}


void SslManager::receiveSockSSL(SSL *ssl, std::string &rcvdMessage, const std::function<void(SSL *ssl, std::string&)> &onMessageReceived) {
    int rslt = 0;
    constexpr int bufferLength{8192};
    std::vector<uint8_t> receiveBuffer(bufferLength);
    try {
        do {
            rslt = SSL_read(ssl, reinterpret_cast<char*>(receiveBuffer.data()), static_cast<int>(bufferLength));
            logger->trace("SslManager::receiveSockSSL(): Receiving data from SSL socket.");
            if (rslt > 0) {
                rcvdMessage.append(reinterpret_cast<char*>(receiveBuffer.data()), rslt);
                logger->trace(std::format("SslManager::receiveSockSSL(): Received {} from SSL socket.", rslt));
                if (rcvdMessage.find("\r\n\r\n") != std::string::npos) { // if message contains \r\n
                    if (rcvdMessage.find("User-Agent:") != std::string::npos) {
                        onMessageReceived(ssl, rcvdMessage); // solution with callback call
                    }
                }
            }else{
                int sslError = SSL_get_error(ssl, rslt);
                if(sslError == SSL_ERROR_ZERO_RETURN){
                    logger->trace("SslManager::receiveSockSSL(): SSL connection close gracefully.");
                    break;
                }else if(sslError == SSL_ERROR_WANT_READ || sslError == SSL_ERROR_WANT_WRITE){
                    std::this_thread::sleep_for(std::chrono::milliseconds(1)); // non-blocking socket, wait and try again.
                    continue;
                }else if (rslt == 0) { // client connection closing
                    logger->error("SslManager::receiveSockSSL(): Client closed socket connection.");
                    break;
                }else {
                    unsigned long errCode = ERR_get_error();
                    std::string errStr = ERR_error_string(errCode, nullptr);
                    std::string exMsg = std::format("SslManager::receiveSockSSL(): Client SSL data receive error: {}", errStr);
                    logger->error(exMsg);
                    throw std::runtime_error(exMsg);
                }
            }
        } while (rslt > 0);
    }catch(const std::exception &ex){
        std::string exMsg = std::format("SslManager::receiveSockSSL(): Unknown exception: {}", ex.what());
        logger->error(exMsg);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        throw;
    }catch(...){
        std::string exMsg = std::format("SslManager::receiveSockSSL(): Unknown exception.");
        logger->error(exMsg);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        throw;
    }
}

    void SslManager::setCertificateFile(const std::string &certFile){ certificateFile = certFile; }
	void SslManager::setPrivateKeyFile(const std::string &keyFile){ privateKeyFile = keyFile; }
