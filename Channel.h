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
        // 消费者使用std::unique_lock来加锁，以便在等待条件变量时释放锁
        // 如果其他线程持有锁，消费者会在这里等待
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty(); });
        T data = queue.front();
        queue.pop();
        return data;
    }

    /*  条件变量：
     *  加锁互斥锁：线程在检查条件之前必须先获取一个互斥锁，以确保条件的检查和条件变量的等待是原子操作。
        等待条件：线程调用条件变量的wait方法等待某个条件。这一步会原子地释放之前获得的互斥锁并使线程进入等待状态。原子性是关键，它确保了互斥锁的释放和线程进入等待状态之间没有时间间隔，避免了竞争条件。
        自动解锁并进入等待状态：当调用wait时，线程会释放互斥锁并等待其他线程在相同条件变量上调用notify_one或notify_all，以通知等待的线程条件可能已变为真。
        被唤醒后重新加锁：一旦条件变量被通知，等待的线程被唤醒并自动重新获取之前释放的互斥锁。然后，它会重新检查条件是否满足，如果不满足，它可能会再次等待。
     */

    // 获取通道名称
    std::string getName() const {
        return name;
    }
};

#endif // CHANNEL_H