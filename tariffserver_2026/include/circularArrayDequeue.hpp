#pragma once
#include <iostream>
#include <stdexcept>
#include <atomic>


template <typename T, size_t capacity>
class circularQueue {
    static_assert((capacity & (capacity - 1)) == 0, "capacity must be a power of 2!");

private:
    T* buffer;
    size_t head;
    size_t tail;
    std::atomic<size_t> count;
    const size_t mask;

public:
    circularQueue() : 
        buffer{new T[capacity]}, 
        head{0}, 
        tail{0},
        count{0},
        mask{capacity - 1}
    {}
    circularQueue(const circularQueue&) = delete;
    circularQueue& operator=(const circularQueue&) = delete;

    circularQueue(circularQueue&& other) noexcept :
        buffer{other.buffer},
        head{other.head},
        tail{other.tail},
        count{other.count.load()},
        mask{other.mask}
    {
        other.buffer = nullptr;
        other.count = 0;
    }

    circularQueue& operator=(circularQueue&& other) noexcept{
        if(this != &other){
            delete[] buffer;
            buffer = other.buffer;
            head = other.head;
            tail = other.tail;
            count = other.count.load();
            other.buffer = nullptr;
            other.count = 0;
        }
        return *this;
    }
   

    ~circularQueue() { delete[] buffer; }


    void enqueue(const T &item) {
        if(!buffer) { throw std::runtime_error("Queue was moved!"); }
        if (isFull()) { throw std::overflow_error("Queue is full"); }
        buffer[tail] = item;
        tail = (tail + 1) & mask;
        count++;
    }


    T dequeue() {
        if (isEmpty()) { throw std::underflow_error("Queue is empty"); }
        T item = std::move(buffer[head]);
        head = (head + 1) & mask;
        count--;
        return item;
    }


    void printQueue() const {
        std::cout << "Queue: ";
        for (size_t i = 0; i < count; i++) {
            size_t index = (head + i) & mask;
            std::cout << buffer[index] << " ";
        }
        std::cout << std::endl;
    }

    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }

    size_t getSize() const { return count; }
    bool isEmpty() const { return count == 0; }
    bool isFull() const { return count == capacity; }
};
