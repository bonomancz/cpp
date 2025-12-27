#include "../include/socketManager.hpp"
#include "../include/logManager.hpp"


SocketManager::SocketManager(std::shared_ptr<LogManager> lgMgr) : logger(lgMgr){}


SocketManager::~SocketManager(){
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
}

void SocketManager::initWSA() {
    this->initializeWinSock2API();
}

void SocketManager::build() {
    this->initialize();
    this->setup();
    this->bindSock();
    this->listenSock();
}

void SocketManager::initializeWinSock2API(){
    int rslt = WSAStartup(MAKEWORD(2, 2), &wsaData);
    logger->trace("SocketManager::initWsockAPI(): Initializing Winsock2 API.");
    if (rslt != 0) {
        throw std::runtime_error("SocketManager::initWsockAPI(): WSAStartup failed with error: " + std::to_string(rslt));
    }
}

void SocketManager::initialize() {
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // resolve address, port
    int rslt = getaddrinfo(NULL, std::to_string(serverPort).c_str(), &hints, &result);
    logger->trace("SocketManager::initialize(): Resolving address, port.");
    if (rslt != 0) {
        throw std::runtime_error("SocketManager::initialize(): getaddrinfo failed with error: " + std::to_string(rslt));
    }
}

void SocketManager::setup() {
    this->serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    logger->trace("SocketManager::setup(): Socket setup.");
    if (this->serverSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        throw std::runtime_error("SocketManager::setup(): Socket failed with error: " + std::to_string(WSAGetLastError()));
    }
}

void SocketManager::bindSock() {
    int rslt = bind(this->serverSocket, result->ai_addr, (int)result->ai_addrlen);
    logger->trace("SocketManager::bindSock(): Socket bind.");
    freeaddrinfo(result);
    if (rslt == SOCKET_ERROR) {
        closesocket(this->serverSocket);
        throw std::runtime_error("SocketManager::bindSock(): Bind failed with error: " + std::to_string(WSAGetLastError()));
    }
}

void SocketManager::listenSock() {
    int rslt = listen(this->serverSocket, SOMAXCONN);
    logger->trace("SocketManager::listenSock(): Listening socket.");
    if (rslt == SOCKET_ERROR) {
        closesocket(this->serverSocket);
        throw std::runtime_error("SocketManager::listenSock(): Listen failed with error: " + std::to_string(WSAGetLastError()));
    }
}

SOCKET SocketManager::acceptSock() {
    SOCKET output = accept(serverSocket, NULL, NULL);
    logger->trace("SocketManager::acceptSock(): Accepting connection.");
    if (output == INVALID_SOCKET) {
        closesocket(serverSocket);
        throw std::runtime_error("SocketManager::acceptSock(): Accept failed with error: " + std::to_string(WSAGetLastError()));
    }
    return output;
}

void SocketManager::receiveSock(SOCKET &clientSocket, std::string &rcvdMessage, const std::function<void(SOCKET&, std::string&)> &onMessageReceived) {
    int rslt = 0;
    constexpr int bufferLength{8192};
    std::vector<uint8_t> receiveBuffer(bufferLength);
    try {
        do {
            rslt = recv(clientSocket, reinterpret_cast<char*>(receiveBuffer.data()), static_cast<int>(bufferLength), 0);
            logger->trace("SocketManager::receiveSock(): Receiving data from socket.");
            if (rslt > 0) {
                rcvdMessage.append(reinterpret_cast<char*>(receiveBuffer.data()), rslt);
                if (rcvdMessage.find("\r\n\r\n") != std::string::npos) { // if message contains \r\n
                    if (rcvdMessage.find("User-Agent:") != std::string::npos) {
                        onMessageReceived(clientSocket, rcvdMessage); // solution with callback call
                    }
                }
            }
            else if (rslt == 0) { // client connection closing
                logger->error("SocketManager::receiveSock(): Client closed socket connection.");
                break;
            }
            else {
                std::string exMsg = std::format("SocketManager::receiveSock(): Client data receive error: {}", WSAGetLastError());
                logger->error(exMsg);
                throw std::runtime_error(exMsg);
            }
        } while (rslt > 0);
    }
    catch(...){
        closesocket(clientSocket);
        std::string exMsg = std::format("SocketManager::receiveSock(): Unknown error: {}", WSAGetLastError());
        logger->trace(exMsg);
        throw;
    }
}

void SocketManager::sendSock(SOCKET& clientSocket, std::string& sendMessage) {
    int rslt = send(clientSocket, sendMessage.data(), static_cast<int>(sendMessage.length()), 0);
    logger->trace("SocketManager::sendSock(): Sending data from socket.");
    if (rslt == SOCKET_ERROR) {
        closesocket(clientSocket);
        throw std::runtime_error("SocketManager::sendSock(): Send failed with error: " + std::to_string(WSAGetLastError()));
    }
}

void SocketManager::shutdownSock(SOCKET &clientSocket) {
    int rslt = shutdown(clientSocket, SD_SEND);
    logger->trace("SocketManager::shutdownSock(): Socket shutdown.");
    if (rslt == SOCKET_ERROR) {
        closesocket(clientSocket);
        throw std::runtime_error("SocketManager::shutdownSock(): Shutdown failed with error: " + std::to_string(WSAGetLastError()));
    }
}

std::string SocketManager::getClientIpAddr(SOCKET& clientSocket) {
    logger->trace("SocketManager::getClientIpAddr(): Getting client IP address.");
    std::string output = "";
    sockaddr_in clientIpAddr;
    int addrSize = sizeof(clientIpAddr);
    char clientIP[INET_ADDRSTRLEN];
    if (getpeername(clientSocket, (sockaddr*)&clientIpAddr, &addrSize) == 0) {
        inet_ntop(AF_INET, &clientIpAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        output = std::string(clientIP);
    }
    return output;
}

void SocketManager::setServerPort(int serverPort) {
    logger->trace(std::format("SocketManager::setServerPort(): Setting server listen port: {}.", serverPort));
    this->serverPort = serverPort;
}

std::string SocketManager::getServerHostname(){
    std::vector<char> host(512);
    std::string hostName;
    if(gethostname(host.data(), static_cast<int>(host.size())) == SOCKET_ERROR){
        std::string exMsg = std::format("SocketManager::getServerHostname(): gethostname() failed with error: {}.", WSAGetLastError());
        logger->error(exMsg);
        throw std::runtime_error(exMsg);
    }
    hostName = std::string(host.data());
    return hostName;
}
