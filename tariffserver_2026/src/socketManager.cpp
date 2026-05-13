#include "socketManager.hpp"
#include "logManager.hpp"
#include "EpollManager.hpp"


SocketManager::SocketManager(std::shared_ptr<LogManager> lgMgr, std::shared_ptr<EpollManager> epMgr) :
    logger{lgMgr},
    epoll{epMgr},
    serverPort{9002},
    serverSocket{-1}
{}


SocketManager::~SocketManager(){ this->stop(); }


void SocketManager::build() {
    this->initialize();
    this->setup();
    this->bindSock();
    this->listenSock();
}


void SocketManager::start() {
    logger->debug("SocketManager starting.");
    this->build();
}


void SocketManager::stop() {
    if(socketManagerRunningFlag.load()){
        socketManagerRunningFlag.store(false);
        logger->debug("SocketManager::stop(): Sending wakeup signal.");
        epoll->wakeup();

        if(serverSocket != -1) {
            logger->debug("SocketManager stopping.");
            shutdown(serverSocket, SHUT_RDWR);
            close(serverSocket);
            serverSocket = -1;
        }
    }
}


void SocketManager::initialize() {
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // resolve address, port
    int rslt = getaddrinfo(NULL, std::to_string(serverPort).c_str(), &hints, &result);
    std::string msg = std::format("SocketManager::initialize(): Resolving address, port ({}).", serverPort);
    logger->trace(msg);
    if (rslt != 0) {
        throw std::runtime_error("SocketManager::initialize(): getaddrinfo failed with error: " + std::string(strerror(rslt)));
    }
}


void SocketManager::setup() {
    this->serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    logger->trace("SocketManager::setup(): Socket setup.");
    if (this->serverSocket < 0) {
        freeaddrinfo(result);
        throw std::runtime_error("SocketManager::setup(): Socket failed with error: " + std::string(strerror(errno)));
    }

    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1 || fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        logger->error("SocketManager::setup(): Failed to set non-blocking on server socket.");
    }

    // setting socket options - for Linux
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        logger->error("SocketManager::setup(): setsockopt(SO_REUSEADDR) failed.");
    }
    logger->trace("SocketManager::setup(): Socket created successfully (fd: " + std::to_string(serverSocket) + ").");
}


void SocketManager::bindSock() {
    int rslt = bind(this->serverSocket, result->ai_addr, (int)result->ai_addrlen);
    logger->trace("SocketManager::bindSock(): Socket bind.");
    freeaddrinfo(result);
    this->result = nullptr;
    if (rslt < 0) {
        close(this->serverSocket);
        throw std::runtime_error("SocketManager::bindSock(): Bind failed with error: " + std::string(strerror(errno)));
    }
}


void SocketManager::listenSock() {
    int rslt = listen(this->serverSocket, SOMAXCONN);
    logger->trace("SocketManager::listenSock(): Listening socket.");
    if (rslt < 0) {
        close(this->serverSocket);
        throw std::runtime_error("SocketManager::listenSock(): Listen failed with error: " + std::string(strerror(errno)));
    }
}


std::vector<int> SocketManager::acceptSock() {
    std::vector<int> acceptedFDs;
    
    while (true) {
        // Přijmeme spojení (v Edge-Triggered módu musíme v cyklu)
        int clientFd = accept(serverSocket, nullptr, nullptr);

        if (clientFd == -1) {
            // EAGAIN/EWOULDBLOCK znamená "všichni odbaveni"
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; 
            }
            logger->error(std::format("SocketManager::acceptSock(): Accept failed: {}", std::string(strerror(errno))));
            break;
        }

        // non-blocking socket, a must
        int flags = fcntl(clientFd, F_GETFL, 0);
        if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1) {
            logger->error(std::format("SocketManager::acceptSock(): Failed to set non-blocking on FD {}", clientFd));
            close(clientFd);
            continue;
        }

        logger->trace(std::format("SocketManager::acceptSock(): Accepted new connection on FD {}", clientFd));
        acceptedFDs.push_back(clientFd);
    }

    return acceptedFDs;
}


void SocketManager::registerClient(int clientSocket) {
    logger->trace("SocketManager::registerClient(): Registering new connected client.");
    if(connectedClients.find(clientSocket) != connectedClients.end()){
        logger->warning(std::format("SocketManager::registerClient(): {} is already registered.", clientSocket));
    }
    auto context = std::make_unique<clientContext>(clientSocket);
    context->ipAddress = getClientIpAddr(clientSocket);
    context->lastActivity = std::chrono::steady_clock::now();
    connectedClients[clientSocket] = std::move(context);
    logger->debug(std::format("SocketManager::registerClient(): New registered client with FD: {}.", clientSocket));
}


void SocketManager::removeClient(int clientSocket) {
    logger->trace("SocketManager::removeClient(): Removing client.");
    auto it = connectedClients.find(clientSocket);
    if (it != connectedClients.end()) {
        close(clientSocket);
        connectedClients.erase(it);
        logger->debug(std::format("SocketManager::removeClient(): Client with FD: {} removed.", clientSocket));
    }
}


void SocketManager::networkEventLoop() {
    const int MAX_EVENTS = 256;
    struct epoll_event events[MAX_EVENTS];

    logger->info("tariffServerMain::networkEventLoop(): Starting network high-performance loop.");

    while (socketManagerRunningFlag.load()) {
        int nfds = epoll->wait(events, MAX_EVENTS, 1000);

        if (nfds == -1) {
            if (errno == EINTR) continue;
            logger->error(std::format("epoll_wait failed: {}", strerror(errno)));
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            // 1. KROK: Rozlišení, jestli je to server nebo klient
            // Server socket byl do epollu přidán jako FD, takže .fd je platné.
            if (events[i].data.fd == getServerSocket()) {
                logger->info("New incoming connection on server socket."); 
                std::vector<int> newClients = acceptSock();
                
                for (int clientFd : newClients) {
                    registerClient(clientFd);
                    auto* ctx = getClientContext(clientFd); 
                    // TADY: Registrujeme přes PTR (vizitku), odteď pro tento socket .fd v unii ignoruj!
                    epoll->addSocket(clientFd, EPOLLIN | EPOLLET, ctx);
                    logger->info(std::format("Client registered on FD {}", clientFd));
                }
                continue; // Vyřízeno, jdeme na další událost v poli
            }

            // 2. KROK: Pokud to není server, je to VŽDY klient (používáme PTR)
            clientContext* ctx = static_cast<clientContext*>(events[i].data.ptr);
            if (!ctx) continue;

            uint32_t revents = events[i].events;

            // SCÉNÁŘ B: Příchozí data
            if (revents & EPOLLIN) {
                logger->debug(std::format("Client data ready on FD {}.", ctx->fd));
                onClientDataReady(ctx);
            } 
            
            // SCÉNÁŘ C: Chyba nebo odpojení
            // Tady používáme ctx->fd, protože to je to pravé číslo socketu,
            // které jsme si schovali do kontextu předtím, než jsme přepsali unii.
            else if (revents & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                int realFd = ctx->fd; 
                logger->debug(std::format("Client on FD {} disconnected.", realFd));
                
                // Úklid (removeClient by měl ideálně volat i epoll->removeSocket)
                removeClient(realFd);
                epoll->removeSocket(realFd);
                close(realFd);
            }
        }
    }
}


void SocketManager::onClientDataReady(clientContext* ctx) {
    if (!ctx) return;
    const int recvBufferSize = 8192;
    std::vector<uint8_t> recvBuffer(recvBufferSize);
    bool connectionClosed = false;
    
    while (true){
        ssize_t bytesRead = recv(ctx->fd, recvBuffer.data(), recvBuffer.size(), MSG_DONTWAIT);
        if(bytesRead > 0) {
            // 1. Uložíme data do tvého vektoru
            ctx->buffer.insert(ctx->buffer.end(), recvBuffer.begin(), recvBuffer.begin() + bytesRead);

            // 2. Aktualizujeme čas poslední aktivity
            ctx->lastActivity = std::chrono::steady_clock::now();
        }else if(bytesRead == 0) {
            connectionClosed = true; // Klient regulérně zavřel spojení
            break;
        }else{
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // Vše vyčteno, končíme cyklus
            }
            if (errno == EINTR){ continue; } // Přerušeno signálem, zkusíme znova            
            logger->error(std::format("Chyba recv na FD {}: {}", ctx->fd, strerror(errno)));
            connectionClosed = true;
            break;
        }
    }

    if (!ctx->buffer.empty()){
        // Vytvoříme kopii/přesun dat pro worker, abychom uvolnili I/O vlákno
        std::vector<uint8_t> dataToProcess = std::move(ctx->buffer);
        ctx->buffer.clear(); 
        if(onClientDataReceivedCallback){
            onClientDataReceivedCallback(ctx->fd, std::string_view(reinterpret_cast<const char*>(dataToProcess.data()), dataToProcess.size()));
        }
    }

    if (connectionClosed) {
        logger->warning(std::format("Disconnecting client {} (FD {})", ctx->ipAddress, ctx->fd));
        removeClient(ctx->fd);
        return;
    }
}


void SocketManager::setOnClientDataReceivedCallback(std::function<void(int, std::string_view)> callback) {
    onClientDataReceivedCallback = std::move(callback);
}


void SocketManager::shutdownSock(int clientSocket) {
    int rslt = shutdown(clientSocket, SHUT_WR);
    logger->trace("SocketManager::shutdownSock(): Socket shutdown.");
    if (rslt < 0) {
        std::string msg = std::format("SocketManager::shutdownSock(): Shutdown failed with error: {}: {}", errno, strerror(errno));
        logger->error(msg);
        throw std::runtime_error(msg);
    }
    if(close(clientSocket) < 0){
        std::string msg = std::format("SocketManager::shutdownSock(): Closing client socket failed with error: ", errno, strerror(errno));
        logger->error(msg);
        throw std::runtime_error(msg);
    }
}


std::string SocketManager::getClientIpAddr(int clientSocket) {
    logger->trace("SocketManager::getClientIpAddr(): Getting client IP address.");
    std::string output = "";
    sockaddr_in clientIpAddr;
    std::memset(&clientIpAddr, 0, sizeof(clientIpAddr));
    socklen_t addrSize = sizeof(clientIpAddr);
    char clientIP[INET_ADDRSTRLEN];
    if (getpeername(clientSocket, (sockaddr*)&clientIpAddr, &addrSize) == 0) {
        inet_ntop(AF_INET, &clientIpAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        output = std::string(clientIP);
    }else{
        std::string msg = std::format("SocketManager::getClientIpAddr(): Getting client ip address failed with error: ", errno, strerror(errno));
        logger->error(msg);
        throw std::runtime_error(msg);
    }
    return output;
}

void SocketManager::setServerPort(int serverPort) {
    logger->trace(std::format("SocketManager::setServerPort(): Setting server listen port: {}.", std::string(strerror(errno))));
    this->serverPort = serverPort;
}

std::string SocketManager::getServerHostname(){
    std::vector<char> host(512);
    std::string hostName;
    if(gethostname(host.data(), static_cast<int>(host.size())) == 0){
        std::string exMsg = std::format("SocketManager::getServerHostname(): gethostname() failed with error: {}.", std::string(strerror(errno)));
        logger->error(exMsg);
        throw std::runtime_error(exMsg);
    }
    hostName = std::string(host.data());
    return hostName;
}


int SocketManager::getServerSocket() const { return serverSocket; }


clientContext* SocketManager::getClientContext(int clientSocket) {
    auto it = connectedClients.find(clientSocket);
    if (it != connectedClients.end()) {
        // it->second je std::unique_ptr<clientContext>
        // .get() vrátí surový ukazatel (raw pointer) na ten objekt
        return it->second.get();
    }    
    logger->error(std::format("SocketManager::getClientContext(): Client with FD {} not found!", clientSocket));
    return nullptr;
}