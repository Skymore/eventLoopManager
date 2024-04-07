#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <functional>
#include <memory>
#include "Event.h"
#include "Channel.h"

class Manager;

using EventHandler = std::function<void(std::shared_ptr<Event>)>;

template<typename T>
using ChannelHandler = std::function<void(T)>;

class Process {
private:
    std::string name;

public:
    Process();

    explicit Process(const std::string name);

    static void subscribeEvent(const std::string &eventType, const EventHandler &handler);

    void publishEvent(const std::string &eventType, std::shared_ptr<Event> event);

    template<typename T>
    void subscribeChannel(const std::string &channelName, const ChannelHandler<T> &handler);

    template<typename T>
    void sendtoChannel(const std::string &channelName, const T &data);
};

// Process类的默认构造函数
inline Process::Process() : name("Unnamed") { }

// Process类的带参数构造函数
inline Process::Process(const std::string name) : name(name) {}

void Process::subscribeEvent(const std::string &eventType, const EventHandler &handler) {
    Manager &manager = Manager::getInstance();
    manager.subscribeEvent(eventType, handler);
}

void Process::publishEvent(const std::string &eventType, std::shared_ptr<Event> event) {
    Manager &manager = Manager::getInstance();
    manager.publishEvent(eventType, event);
}

// 订阅channel
// 通过Manager类的getOrCreateChannel方法获取或创建Channel对象，然后创建一个任务，该任务从Channel对象中接收数据并调用handler函数处理数据
template<typename T>
void Process::subscribeChannel(const std::string &channelName, const ChannelHandler<T>& handler) {
    Manager& manager = Manager::getInstance();
    manager.subscribeChannel(channelName, handler);
}


template<typename T>
void Process::sendtoChannel(const std::string &channelName, const T &data) {
    Manager &manager = Manager::getInstance();
    manager.publishToChannel(channelName, data);
}

#endif // PROCESS_H
