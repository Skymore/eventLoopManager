// Consumer.h
#ifndef CONSUMER_H
#define CONSUMER_H

#include <iostream>
#include <string>
#include <condition_variable>
#include <mutex>
#include "Process.h"
#include "Event.h"
#include "kafkaProducer.h"
#include "StatusChangeEvent.h"

class Consumer {
public:
    std::condition_variable cv;
    std::mutex mtx;
    bool ready = false;
    std::string name_;
    std::string currentStatus = "Pause";
    KafkaProducer kafkaProducer;

    Consumer(const std::string &name, const std::string &configFile, const std::string &topic) : name_(name),
                                                                                                 kafkaProducer(
                                                                                                         configFile,
                                                                                                         topic) {}

    void consumeData() {
        Process process(name_);

        // 订阅通道，接收数据
        /*
         * 这段代码位于消费者类中，是用来订阅并接收来自 "DataChannel" 通道的数据。这里用到了 C++11 的 lambda 表达式作为回调函数，用于处理接收到的数据。下面我会分步解释这段代码的作用，然后讨论 currentStatus 变量的定义位置问题。
            代码解释
            订阅通道： process.subscribeChannel<std::string>("DataChannel", ...); 这一行订阅了名为 "DataChannel" 的通道。这意味着，当有数据发送到这个通道时，提供给 subscribeChannel 的回调函数会被调用。
            回调函数： 回调函数通过 lambda 表达式定义，形式为 [this](const std::string& data) {...}。这个函数捕获了当前对象指针 this，以便在函数内部访问消费者的状态和成员变量。
            等待条件满足： std::unique_lock<std::mutex> lock(this->mtx); this->cv.wait(lock, [this, &currentStatus]{ return this->ready || this->finished; });
                这两行代码使用了条件变量来暂停线程的执行，直到 ready 变为 true 或者 finished 变为 true。这是为了控制何时处理接收到的数据：只有在准备好接收时（ready 为 true）或者准备结束（finished 为 true），线程才会继续执行。
            处理接收到的数据： 如果 finished 标志被设置为 true，则函数立即返回，不再处理数据。这是一种清理或结束操作的标志。如果没有结束，则输出接收到的数据，并可能进行进一步的处理，如 JSON 解析。
         */
        process.subscribeChannel<std::string>("DataChannel", [this](const std::string &data) {
            std::unique_lock<std::mutex> lock(this->mtx);
            this->cv.wait(lock, [this] { return this->ready; });


            // 输出接收到的数据(示例用途)
            // std::cout << this->name_ << " receives data: " << data << std::endl;

            // 温湿度阈值控制，低于一定温度打开加热器，低于一定湿度打开加湿器
            double temperatureThreshold = 10.0;
            double humidityThreshold = 30.0;
            if (data.find("temperature") != std::string::npos && data.find("humidity") != std::string::npos) {
                rapidjson::Document doc;
                doc.Parse(data.c_str());
                double temperature = doc["temperature"].GetDouble();
                double humidity = doc["humidity"].GetDouble();
                if (temperature < temperatureThreshold) {
                    std::cout << "Temperature is too low. Turn on the heater." << std::endl;
                }
                if (humidity < humidityThreshold) {
                    std::cout << "Humidity is too low. Turn on the humidifier." << std::endl;
                }
            }


            // 将收到的数据发送到Kafka
            kafkaProducer.produce(data);

            // 输出发送到Kafka的数据(示例用途)
            // std::cout << this->name_ << " sent data to Kafka: " << data << std::endl;
        });

        // 订阅状态变化事件
        process.subscribeEvent("StatusChangeEvent", [this](std::shared_ptr<Event> event) {
            if (auto statusChangeEvent = std::dynamic_pointer_cast<StatusChangeEvent>(event)) {
                std::unique_lock<std::mutex> lock(this->mtx);
                currentStatus = statusChangeEvent->status;
                if (currentStatus == "Receive") {
                    this->ready = true;
                } else {
                    this->ready = false;
                }
                this->cv.notify_all();
                std::cout << this->name_ << " receives status change event: " << currentStatus << std::endl;
            }
        });

    }

    ~Consumer() {
        std::cout << "Consumer " << name_ << " is destroyed." << std::endl;
    }

};

#endif // CONSUMER_H
