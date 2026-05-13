#pragma once
#include <sys/epoll.h>
#include <vector>
#include <stdexcept>
#include <unistd.h>


class EpollManager {
private:
    int epoll_fd = epoll_create1(0);
    int stop_event_fd;
public:
    EpollManager();
    // EpollManager(const EpollManager&) = delete;
    // EpollManager& operator=(const EpollManager&) = delete;
    // EpollManager(EpollManager&&) = delete;
    // EpollManager& operator=(EpollManager&&) = delete;
    ~EpollManager();
    void addSocket(int fd, uint32_t events, void* ptr = nullptr);
    int wait(struct epoll_event* events, int max_events, int timeout);
    void removeSocket(int fd);
    int get_fd() const;
    void wakeup();
    int getStopEventFd() const;
};