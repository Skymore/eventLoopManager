#ifndef CHANNEL_H
#define CHANNEL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>

template<typename T>
class Channel {
private:
    std::string name;
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cv;

public:
    Channel() : name("Unnamed") {}

    explicit Channel(const std::string &name) : name(name) {}

    void send(const T &data) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(data);
//        std::cout << "Sent data to channel: " << data << std::endl;
        cv.notify_one();
    }

    T receive() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty(); });
        T data = queue.front();
        queue.pop();
        return data;
    }

    // 获取通道名称
    std::string getName() const {
        return name;
    }
};

#endif // CHANNEL_H