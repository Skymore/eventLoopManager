#include "KafkaProducer.h"
#include <fstream>
#include <sstream>
#include <iostream>

KafkaProducer::KafkaProducer(const std::string& configFile, const std::string& topicStr)
        : producer(nullptr), topic(nullptr), topicStr(topicStr) {
    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    std::ifstream configFileStream(configFile);
    std::string line;

    std::cout << "Reading Kafka configuration from file: " << configFile << std::endl;
    if (!configFileStream.is_open()) {
        std::cerr << "Failed to open Kafka configuration file: " << configFile << std::endl;
        exit(1);
    }

    while (getline(configFileStream, line)) {
        std::istringstream lineStream(line);
        std::string key, val;
        if (getline(lineStream, key, '=') && getline(lineStream, val)) {
            std::cout << "Setting Kafka configuration: " << key << " = " << val << std::endl;
            if (conf->set(key, val, errstr) != RdKafka::Conf::CONF_OK) {
                std::cerr << "Failed to set Kafka configuration: " << errstr << std::endl;
                exit(1);
            }
        }
    }

    producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        exit(1);
    }

    RdKafka::Conf* topicConf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    topic = RdKafka::Topic::create(producer, topicStr, topicConf, errstr);
    if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << std::endl;
        exit(1);
    }

    delete conf;
    delete topicConf;
}

KafkaProducer::~KafkaProducer() {
    // 确保所有排队的消息都被发送完毕
    producer->flush(50000); // 参数是等待的毫秒数，这里假设等待最长5秒

    if (topic) {
        delete topic; // 删除 topic 对象
        topic = nullptr; // 避免野指针
    }

    if (producer) {
        delete producer; // 删除 producer 对象
        producer = nullptr; // 避免野指针
    }
}

void KafkaProducer::produce(const std::string& message) {
    RdKafka::ErrorCode resp;
    std::string errstr;

    // 使用 librdkafka 发送消息。这里假设 topicStr 和 producer 已在构造函数中正确初始化
    resp = producer->produce(
            topic,                             // topic 对象
            RdKafka::Topic::PARTITION_UA,      // 使用未分配的分区
            RdKafka::Producer::RK_MSG_COPY,    // 消息内容需要被 librdkafka 复制
            const_cast<char *>(message.c_str()), message.size(), // 消息内容和长度
            NULL,                              // 没有 key
            NULL);                             // Opaque value (用户提供的数据，回调时返回)

    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Produce failed: " << RdKafka::err2str(resp) << std::endl;
    } else {
        // std::cout << "Message produced: " << message << std::endl;
    }

    // librdkafka 基于异步的，调用 poll 来触发回调（如消息发送成功或失败的回调）
    producer->poll(0);
}
