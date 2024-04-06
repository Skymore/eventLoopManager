//
// Created by 陶睿 on 4/1/24.
//

#ifndef EVENTLOOPMANAGER_THREADSAFEWRITEPRIORITYQUEUE_H
#define EVENTLOOPMANAGER_THREADSAFEWRITEPRIORITYQUEUE_H

#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include "ThreadSafeQueueInterface.h"

template<typename T>
class ThreadSafeWritePriorityQueue : public ThreadSafeQueueInterface<T> {
private:
    std::queue<T> que;
    mutable std::shared_mutex mtx;
    mutable std::condition_variable_any readCond, writeCond;
    int writeWaitingCount = 0; // 记录等待写入的线程数

public:
    ThreadSafeWritePriorityQueue() = default;   // 默认构造函数
    ~ThreadSafeWritePriorityQueue() = default;

    void push(T value) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        writeWaitingCount++;
        que.emplace(value);  // 使用 emplace 和 std::forward实现完美转发
        writeWaitingCount--;
        readCond.notify_one();
        writeCond.notify_all();
    }

    void pop() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        writeCond.wait(lock, [this] { return que.empty() || writeWaitingCount == 0; }); // 如果队列为空或者等待写的线程数为 0
        if (que.empty()) {
            throw std::runtime_error("pop from empty queue");
        }
        T value = std::move(que.front());
        que.pop();
        if (que.empty() || writeWaitingCount > 0) {
            writeCond.notify_one();
        } else {
            readCond.notify_one();
        }
    }

    T waitAndPop() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        readCond.wait(lock, [this] { return !que.empty(); }); // 等待直到队列非空
        T value = std::move(que.front());
        que.pop();
        if (writeWaitingCount > 0) {
            // 优先唤醒正在等待写入的线程
            writeCond.notify_one();
        }
        return value;
    }


    T front() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("front from empty queue");
        }
        return que.front();
    }

    T waitAndFront() const {
        std::unique_lock<std::shared_mutex> lock(mtx);
        readCond.wait(lock, [this] { return !que.empty(); });
        return que.front();
    }

    T back() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (que.empty()) {
            throw std::runtime_error("back from empty queue");
        }
        return que.back();
    }

    T waitAndBack() const {
        std::unique_lock<std::shared_mutex> lock(mtx);
        readCond.wait(lock, [this] { return !que.empty(); });
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


#endif //EVENTLOOPMANAGER_THREADSAFEWRITEPRIORITYQUEUE_H
