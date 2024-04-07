#include <iostream>
#include <chrono>
#include <filesystem> // C++17及以上
#include <fstream>
#include <memory>
#include <thread>
#include <librdkafka/rdkafkacpp.h>
#include "Manager.h"
#include "Process.h"
#include "Producer.h"
#include "Consumer.h"
#include "StatusChangeEvent.h"
#include "Event.h"
#include "ProducerConfig.h"
#include "SensorReader.h"

// 状态改变者类
class StatusChanger {
public:
    void changeStatus() {
        Process process("StatusChanger");

        // 模拟状态变化
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // 发布状态变化事件开始收集数据
        std::string status = "Receive";
        std::shared_ptr<Event> event = std::make_shared<StatusChangeEvent>(status);
        process.publishEvent("StatusChangeEvent", event);
    }
};


int main() {
    Manager &manager = Manager::getInstance();

    // 配置文件路径和Kafka主题
    std::filesystem::path cPath = std::filesystem::current_path();
    std::filesystem::path kafkaConfigPath = cPath.parent_path() / "configs/kafka_config.txt";
    std::filesystem::path sensorsConfigPath = cPath.parent_path() / "configs/sensors_config.json";
    std::string topic = "topic_0";

    // 状态改变者和消费者对象
    StatusChanger statusChanger;
    Consumer consumer("Consumer", kafkaConfigPath, topic);
    //Consumer consumer2("Consumer2", kafkaConfigPath, topic);

    // 加载配置文件
    auto configs = ProducerConfig::loadFromJson(sensorsConfigPath);

    // 生产者线程
    std::vector<std::thread> producerThreads;
    // 为每个配置创建Producer实例
    std::vector<Producer> producers;
    for (const auto &config: configs) {
        producers.emplace_back(config.name, config.latitude, config.longitude);
    }

    // 创建和启动生产者线程
    for (auto &producer: producers) {
        producerThreads.emplace_back(&Producer::produceData, &producer);
    }

    std::cout << "Main thread: " << std::this_thread::get_id() << " is running." << std::endl;

    // 创建和启动状态改变者和消费者线程
    std::thread statusChangerThread(&StatusChanger::changeStatus, &statusChanger);
    std::thread consumerThread(&Consumer::consumeData, &consumer);
    //std::thread consumerThread2(&Consumer::consumeData, &consumer2);

    // 运行 20 秒
    manager.run(std::chrono::seconds(10));

    std::cout << "Main thread: " << std::this_thread::get_id() << " manager stopped." << std::endl;

    // 等待生产者、状态改变者和消费者线程结束
    for (auto &t: producerThreads) {
        t.join();
    }
    statusChangerThread.join();
    consumerThread.join();

    std::cout << "Main thread: " << std::this_thread::get_id() << " all threads joined." << std::endl;


    /*
    // 创建KafkaProducer实例
    KafkaProducer kafkaProducer(configPath, topic);

    // 要发送的消息
    std::string message = "Hello, Kafka2!";

    // 发送消息
    std::cout << "Sending message: " << message << std::endl;
    kafkaProducer.produce(message);

    // 因为librdkafka是异步的，这里简单等待一会儿以便让消息有机会被发送出去
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Message sent." << std::endl;

    // KafkaProducer的析构函数会被自动调用，清理资源
    */

    return 0;
}

