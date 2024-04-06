//
// Created by 陶睿 on 4/1/24.
//


#ifndef EVENTLOOPMANAGER_THREADSAFEBLOCKINGQUEUE_H
#define EVENTLOOPMANAGER_THREADSAFEBLOCKINGQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <exception>
#include "ThreadSafeQueueInterface.h"

template<typename T>
class ThreadSafeBlockingQueue : public ThreadSafeQueueInterface<T>{
private:
    std::queue<T> que;
    mutable std::mutex mtx;
    mutable std::condition_variable cv;
    size_t capacity = 0;

public:
    ThreadSafeBlockingQueue() = default;
    ~ThreadSafeBlockingQueue() = default;

    void push(const T &value) {
        std::lock_guard<std::mutex> lock(mtx);
        que.emplace(value);
        cv.notify_one();
    }

    T waitAndPop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !que.empty(); });
        auto value = std::move(que.front());
        que.pop();
        return value;
    }

    T pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        auto value = std::move(que.front());
        que.pop();
        return value;
    }

    T front() const {
        std::lock_guard<std::mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return que.front();
    }

    T waitAndFront() const {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !que.empty(); });
        return que.front();
    }

    T back() const {
        std::lock_guard<std::mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return que.back();
    }

    T waitAndBack() const {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !que.empty(); });
        return que.back();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return que.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return que.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        std::queue<T> empty;
        std::swap(que, empty);
    }

};


#endif //EVENTLOOPMANAGER_THREADSAFEBLOCKINGQUEUE_H
