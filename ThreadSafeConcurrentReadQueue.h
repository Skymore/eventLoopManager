//
// Created by 陶睿 on 4/1/24.
//



#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <iostream>
#include "ThreadSafeQueueInterface.h"

#ifndef EVENTLOOPMANAGER_THREADSAFECONCURRENTREADQUEUE_H
#define EVENTLOOPMANAGER_THREADSAFECONCURRENTREADQUEUE_H

/*
 * 在多线程编程中，实现一个并发读的队列意味着在并发访问时给予读操作更高的优先级。
 * 通常，这种队列用于读操作远多于写操作的场景，以减少读线程的等待时间。
 * 使用<mutex>、<condition_variable>和读写锁<shared_mutex>来实现一个简单的读优先队列。
 * 使用std::shared_mutex来实现读写锁。std::shared_lock用于读操作，允许多个线程同时读取数据，
 * 而std::unique_lock用于写操作，一次只允许一个线程进行修改。
 */

template<typename T>
class ThreadSafeConcurrentReadQueue : public ThreadSafeQueueInterface<T> {
private:
    std::queue<T> que;
    mutable std::shared_mutex mtx;
    mutable std::condition_variable_any cv;

public:
    ThreadSafeConcurrentReadQueue() = default;
    ~ThreadSafeConcurrentReadQueue() = default;

    void push(T value) {
        std::lock_guard<std::shared_mutex> lock(mtx);
        que.emplace(value);
        cv.notify_one();
    }

    T pop() {
        std::lock_guard<std::shared_mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        auto value = std::move(que.front());
        que.pop();
        return value;
    }

    T waitAndPop() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        cv.wait(lock, [this] { return !que.empty(); });
        auto value = std::move(que.front());
        que.pop();
        return value;
    }

    T front() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return que.front();
    }

    T waitAndFront() const {
        std::unique_lock<std::shared_mutex> lock(mtx);
        cv.wait(lock, [this] { return !que.empty(); });
        return que.front();
    }

    T back() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return que.back();
    }

    T waitAndBack() const {
        std::unique_lock<std::shared_mutex> lock(mtx);
        cv.wait(lock, [this] { return !que.empty(); });
        return que.back();
    }

    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(mtx); // 使用 shared_lock 以允许并发的读操作
        return que.empty();
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mtx); // 同上，允许并发读
        return que.size();
    }

    void clear() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        std::queue<T> empty;
        std::swap(que, empty);
    }
};

#endif //EVENTLOOPMANAGER_THREADSAFECONCURRENTREADQUEUE_H