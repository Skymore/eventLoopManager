#ifndef KAFKAPRODUCER_H
#define KAFKAPRODUCER_H

#include <librdkafka/rdkafkacpp.h>
#include <string>

class KafkaProducer {
public:
    // 构造函数现在接收配置文件路径和主题名称
    KafkaProducer(const std::string& configFile, const std::string& topicStr);
    ~KafkaProducer();
    void produce(const std::string& message);

private:
    RdKafka::Producer* producer;
    RdKafka::Topic* topic;
    std::string topicStr;
};

#endif // KAFKAPRODUCER_H
