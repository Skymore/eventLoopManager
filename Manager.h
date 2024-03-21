#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <memory>
#include "Channel.h"

class Process;

class Event;

using EventHandler = std::function<void(std::shared_ptr<Event>)>;

class Manager {
private:
    Manager() : running(true) {}

    std::map<std::string, std::shared_ptr<void>> channels;
    std::map<std::string, std::shared_ptr<Process>> processes;
    std::map<std::string, std::vector<EventHandler>> eventListeners;
    bool running;
    std::mutex eventMutex;
    std::mutex channelMutex;

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

    void run();

    void stop();
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
            handler(event);
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

void Manager::run() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(20));
    }
}

void Manager::stop() {
    running = false;
}

#endif // MANAGER_H
