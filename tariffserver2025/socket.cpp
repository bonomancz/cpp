#include "socket.h"

void Socket::build() {
    this->initialize();
    this->setup();
    this->bindSock();
    this->listenSock();
}

void Socket::initialize() {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // resolve address, port
    int rslt = getaddrinfo(NULL, std::to_string(serverPort).c_str(), &hints, &result);
    if (rslt != 0) {
        throw std::system_error(errno, std::generic_category(), "Socket::initialize(): getaddrinfo failed with error.");
    }
}

void Socket::setup() {
    this->serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (this->serverSocket == -1) {
        freeaddrinfo(result);
        throw std::system_error(errno, std::generic_category(), "Socket::setup(): Socket failed with error.");
    }
}

void Socket::bindSock() {
    int rslt = bind(this->serverSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);
    if (rslt == -1) {
        close(this->serverSocket);
        throw std::system_error(errno, std::generic_category(), "Socket::bindSock(): Bind failed with error.");
    }
}

void Socket::listenSock() {
    int rslt = listen(this->serverSocket, SOMAXCONN);
    if (rslt == -1) {
        close(this->serverSocket);
        throw std::system_error(errno, std::generic_category(), "Socket::listenSock(): Listen failed with error.");
    }
}

int Socket::acceptSock() {
    int output = accept(this->serverSocket, NULL, NULL);
    if (output == -1) {
        close(serverSocket);
        throw std::system_error(errno, std::generic_category(), "Socket::acceptSock(): Accept failed with error.");
    }
    return output;
}

bool Socket::receiveSock(int clientSocket, std::string& rcvdMessage, std::function<void(int, std::string&)> onMessageReceived, std::string& errorMessage) {
    bool returnValue = true;
    rcvdMessage.clear();
    ssize_t rslt = 0;
    const size_t bufferLength = 8192;
    std::vector<std::uint8_t> receiveBuffer(bufferLength);
    try {
        do {
            if ((rslt = recv(clientSocket, receiveBuffer.data(), bufferLength, 0)) > 0) {
                rcvdMessage.append(reinterpret_cast<const char*>(receiveBuffer.data()), rslt);
                onMessageReceived(clientSocket, rcvdMessage); // solution with callback call
            }
            else if (rslt == 0) { // client connection closing
                throw std::system_error(errno, std::generic_category(), "Socket::receiveSock(): Client closed connection.");
            }
            else {
                throw std::system_error(errno, std::generic_category(), "Socket::receiveSock(): Client data receive error.");
            }
        } while (rslt > 0);
    }
    catch (std::exception& ex) {
        errorMessage = std::string(ex.what());
        returnValue = false;
        throw std::runtime_error(std::string(ex.what()));
    }
    return returnValue;
}

bool Socket::sendSock(int clientSocket, const std::vector<std::uint8_t>& sendMessage) {
    bool returnValue = true;
    try {
        ssize_t bytesSent;
        if ((bytesSent = send(clientSocket, reinterpret_cast<const char*>(sendMessage.data()), static_cast<int>(sendMessage.size()), 0)) == -1) {
            throw std::system_error(errno, std::generic_category(), "sendSock(): Exception: Send failed.");
        }
    }
    catch (std::exception &ex) {
        returnValue = false;
        throw std::runtime_error(std::string(ex.what()));
    }
    return returnValue;
}

void Socket::shutdownSock(int clientSocket) {
    if (shutdown(clientSocket, SHUT_RDWR) == -1 && errno != ENOTCONN) {
        throw std::system_error(errno, std::generic_category(), "Socket::shutdownSock(): Shutdown failed with error.");
    }
    if (close(clientSocket) == -1) {
        throw std::system_error(errno, std::generic_category(), "Socket::shutdownSock(): Close failed with error.");
    }
}

std::string Socket::getClientIpAddr(int clientSocket) {
    std::string output = "";
    sockaddr_in clientIpAddr;
    socklen_t addrSize = sizeof(clientIpAddr);
    char clientIP[INET_ADDRSTRLEN];
    if (getpeername(clientSocket, (sockaddr*)&clientIpAddr, &addrSize) == 0) {
        inet_ntop(AF_INET, &clientIpAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        output = std::string(clientIP);
    }
    return output;
}

void Socket::setServerPort(int serverPort) { this->serverPort = serverPort; }
int Socket::getServerPort() { return this->serverPort; }
