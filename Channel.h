#ifndef CHANNEL_H
#define CHANNEL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>
#include "ThreadSafeBlockingQueue.h"

template<typename T>
class Channel {
private:
    std::string name;
    std::unique_ptr<ThreadSafeQueueInterface<T>> queue;

public:
    // Constructors must now initialize the queue pointer with an instance of a class that implements ThreadSafeQueueInterface
    Channel() : name("Unnamed"), queue(std::make_unique<ThreadSafeBlockingQueue<T>>()) {}

    Channel(const std::string &name) : name(name), queue(std::make_unique<ThreadSafeBlockingQueue<T>>()) {}

    void send(const T &data) {
        queue->push(data);
        std::cout << "Sent data to channel: " << data << std::endl;
    }

    T receive() {
        T data = queue->waitAndPop();
        return data;
    }

    std::string getName() const {
        return name;
    }
};

#endif // CHANNEL_H