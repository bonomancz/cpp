#include "EpollManager.hpp"


EpollManager::EpollManager(){
    epoll_fd = epoll_create1(0);
    if(epoll_fd == -1){
        throw std::runtime_error("EpollManager::EpollManager(): epoll_create1() failed.");
    }
}


EpollManager::~EpollManager(){
    if(epoll_fd != -1){
        close(epoll_fd);
    }
}


void EpollManager::addSocket(int fd, uint32_t events, void* ptr){
    struct epoll_event ev;
    ev.events = events;
    if(ptr){
        ev.data.ptr = ptr;
    }else{
        ev.data.fd = fd;
    }
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1){
        throw std::runtime_error("EpollManager::addSocket(): epoll_ctl() failed.");
    }
}


int EpollManager::wait(struct epoll_event* events, int max_events, int timeout = -1){
    return epoll_wait(epoll_fd, events, max_events, timeout);
}


void EpollManager::removeSocket(int fd){
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
}


int EpollManager::get_fd() const { return epoll_fd; }


void EpollManager::wakeup(){
    uint64_t u = 1;
    write(stop_event_fd, &u, sizeof(uint64_t));
}


int EpollManager::getStopEventFd() const { return stop_event_fd; }

