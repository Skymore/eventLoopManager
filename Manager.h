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
#include "Channel.h"

using namespace std::chrono;

class Process;

class Event;

using EventHandler = std::function<void(std::shared_ptr<Event>)>;

class Manager {
private:
    Manager() = default;

    std::map<std::string, std::shared_ptr<void>> channels;
    std::map<std::string, std::shared_ptr<Process>> processes;
    std::map<std::string, std::vector<EventHandler>> eventListeners;
    // 任务队列 taskQueue
    std::queue<std::pair<std::shared_ptr<Event>, EventHandler>> taskQueue;

    std::mutex eventMutex;
    std::mutex channelMutex;
    high_resolution_clock::time_point _start_time;
    high_resolution_clock::duration _elapsed;

public:
    Manager(const Manager &) = delete;

    Manager &operator=(const Manager &) = delete;

    static Manager &getInstance();

    void subscribeEvent(const std::string &eventType, const EventHandler &handler);

    void publishEvent(const std::string &eventType, std::shared_ptr<Event> event);

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
            taskQueue.emplace(event, handler);
            std::cout << "Task added to queue" << std::endl;
        }
    }
}

template<typename T>
Channel<T> &Manager::getOrCreateChannel(const std::string &channelName) {
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
    _elapsed = high_resolution_clock::duration::zero();

    while ( _elapsed < runtime ) {
        if (taskQueue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        auto [event, handler] = taskQueue.front();
        std::cout << "Processing task" << std::endl;
        handler(event);
        std::cout << "Task processed" << std::endl;
        taskQueue.pop();

        high_resolution_clock::time_point now = high_resolution_clock::now();
        _elapsed = now - _start_time;
    }

}

#endif // MANAGER_H
