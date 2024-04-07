#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <memory>
#include <queue>
#include <chrono>
#include <any>
#include "Channel.h"
#include "ThreadPool.h"

using namespace std::chrono;

class Process;

class Event;

using EventHandler = std::function<void(std::shared_ptr<Event>)>;
template<typename T>
using ChannelHandler = std::function<void(T)>;

class Manager {
private:
    Manager() : threadPool(std::make_unique<ThreadPool>(4)) {} // 假设线程池大小为4

    std::map<std::string, std::shared_ptr<void>> channels;
    std::map<std::string, std::shared_ptr<Process>> processes;
    std::map<std::string, std::vector<EventHandler>> eventListeners;
    std::map<std::string, std::vector<std::function<void(std::any)>>> channelListeners;
    // event任务队列 eventTaskQueue
    std::queue<std::pair<std::shared_ptr<Event>, EventHandler>> eventTaskQueue;

    // channel处理线程池
    std::unique_ptr<ThreadPool> threadPool;

    // event互斥锁
    std::mutex eventMutex;

    // channel互斥锁
    std::mutex channelMutex;

    high_resolution_clock::time_point _start_time;
    high_resolution_clock::duration _elapsed;

public:
    Manager(const Manager &) = delete;

    Manager &operator=(const Manager &) = delete;

    static Manager &getInstance();

    // 提供线程池访问接口
    ThreadPool &getThreadPool() {
        return *threadPool;
    }

    void subscribeEvent(const std::string &eventType, const EventHandler &handler);

    void publishEvent(const std::string &eventType, std::shared_ptr<Event> event);

    template<typename T>
    void subscribeChannel(const std::string &channelName, std::function<void(T)> listener);

    template<typename T>
    void publishToChannel(const std::string &channelName, T data);

    template<typename T>
    Channel<T> &getOrCreateChannel(const std::string &channelName);

    template<typename T>
    void createChannel(const std::string &channelName);

    void run(high_resolution_clock::duration runtime);

};


Manager &Manager::getInstance() {
    static Manager instance;
    return instance;
}

void Manager::subscribeEvent(const std::string &eventType, const EventHandler &handler) {
    std::lock_guard<std::mutex> lock(eventMutex);
    eventListeners[eventType].push_back(handler);
}

void Manager::publishEvent(const std::string &eventType, std::shared_ptr<Event> event) {
    std::lock_guard<std::mutex> lock(eventMutex);
    if (eventListeners.find(eventType) != eventListeners.end()) {
        for (auto &handler: eventListeners[eventType]) {
            // handler(event);
            // 改为事件循环
            eventTaskQueue.emplace(event, handler);
            std::cout << "Task added to queue" << std::endl;
        }
    }
}

template<typename T>
void Manager::subscribeChannel(const std::string &channelName, std::function<void(T)> listener) {
    // 将listener封装为接受std::any的函数，然后存储
    std::function<void(std::any)> anyListener = [listener](std::any data) {
        listener(std::any_cast<T>(data));
    };
    channelListeners[channelName].push_back(anyListener);
}

template<typename T>
void Manager::publishToChannel(const std::string &channelName, T data) {
    if (channelListeners.find(channelName) != channelListeners.end()) {
        for (auto &listener: channelListeners[channelName]) {
            // 将数据封装为std::any类型
            std::any anyData = data;

            // 使用线程池异步执行监听器
            std::cout << "Enqueueing channel listener" << std::endl;
            getThreadPool().enqueue([listener, anyData]() {
                // 在这个lambda表达式中调用监听器
                listener(anyData);
            });

            // 同步调用监听器
//            std::cout << "Calling channel listener" << std::endl;
//            listener(anyData);
        }
    }
}

template<typename T>
Channel<T> &Manager::Manager::getOrCreateChannel(const std::string &channelName) {
    std::lock_guard<std::mutex> lock(channelMutex);
    auto it = channels.find(channelName);
    if (it != channels.end()) {
        return *std::static_pointer_cast<Channel<T>>(it->second);
    } else {
        auto channel = std::make_shared<Channel<T>>(channelName);
        channels[channelName] = channel;
        return *channel;
    }
}

template<typename T>
void Manager::createChannel(const std::string &channelName) {
    std::lock_guard<std::mutex> lock(channelMutex);
    auto channel = std::make_shared<Channel<T>>(channelName);
    channels[channelName] = channel;
}

// 事件循环
void Manager::run(high_resolution_clock::duration runtime) {
    _start_time = high_resolution_clock::now();
    // _elapsed 在循环内部更新

    while (true) { // 修改循环条件
        std::cout << "Event loop running" << std::endl;
        std::cout << "Event queue size: " << eventTaskQueue.size() << std::endl;

        // 在循环内部更新_elapsed，以确保能够获取最新的经过时间
        _elapsed = high_resolution_clock::now() - _start_time;
        std::cout << "Elapsed time: " << duration_cast<seconds>(_elapsed).count() << "s" << std::endl;

        // 检查是否超过了指定的运行时间
        if (_elapsed >= runtime) {
            break; // 终止循环
        }

        if (eventTaskQueue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        auto [event, handler] = eventTaskQueue.front();
        std::cout << "Processing event task" << std::endl;
        std::cout << "StatusChangeEvent " << std::endl;
        handler(event);
        std::cout << "event Task processed" << std::endl;
        eventTaskQueue.pop();
    }
    std::cout << "Event loop stopped" << std::endl;
}


#endif // MANAGER_H
