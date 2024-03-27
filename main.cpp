#include <iostream>
#include <chrono>
#include <random> // 用于生成随机数据
#include <filesystem> // C++17及以上
#include <fstream>
#include <librdkafka/rdkafkacpp.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Manager.h"
#include "Process.h"

// 自定义事件类型
class StatusChangeEvent : public Event {
public:
    std::string status;

    StatusChangeEvent(const std::string &status) : status(status) {}
};

// 生产者类
class Producer {
public:
    Producer(const std::string& name) : producerName(name) {
        // 一开始为每个生产者生成一个随机的经纬度
        latitude = generateRandomLatitude();
        longitude = generateRandomLongitude();
    }

    void produceData() {
        Process process(producerName);

        for (int i = 1; i <= 100; ++i) {
            rapidjson::Document doc;
            doc.SetObject();
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

            double temperature = generateRandomTemperature();
            doc.AddMember("id", i, allocator);
            rapidjson::Value nameValue; // 创建一个空的Value对象
            nameValue.SetString(producerName.c_str(), static_cast<rapidjson::SizeType>(producerName.length()), allocator); // 设置字符串值
            doc.AddMember("name", nameValue, allocator); // 添加到文档
            doc.AddMember("temperature", temperature, allocator);
            doc.AddMember("latitude", latitude, allocator);
            doc.AddMember("longitude", longitude, allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            std::string jsonData = buffer.GetString();

            std::cout << producerName << " sends data: " << jsonData << std::endl;
            process.sendtoChannel("DataChannel", jsonData);
            int time_delta = generateRandomTime();
            std::this_thread::sleep_for(std::chrono::milliseconds(time_delta));
        }
    }

private:
    double latitude; // 生产者的纬度
    double longitude; // 生产者的经度
    std::string producerName; // 生产者名称

    // 生成随机时间，假设在0到500毫秒之间
    static int generateRandomTime() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 500);
        return std::round(dis(gen));
    }

    // 生成随机温度，假设在10到30度之间
    static double generateRandomTemperature() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(10.0, 30.0);
        // 保留两位小数
        return std::round(dis(gen) * 100) / 100;
    }

    // 生成随机纬度，示例假设在-90到90度之间
    static double generateRandomLatitude() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-90.0, 90.0);
        return std::round(dis(gen) * 1000) / 1000;
    }

    // 生成随机经度，示例假设在-180到180度之间
    static double generateRandomLongitude() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-180.0, 180.0);
        return std::round(dis(gen) * 1000) / 1000;
    }
};

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

// 消费者类
class Consumer {
public:
    std::condition_variable cv;
    std::mutex mtx;
    bool ready = false;
    std::string name_;
    std::string currentStatus = "Pause";
    KafkaProducer kafkaProducer;

    Consumer(const std::string& configFile, const std::string& topic) : kafkaProducer(configFile, topic) { }

    void consumeData(const std::string &name) {
        this->name_ = name;
        Process process(name);

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
            std::cout << this->name_ << " receives data: " << data << std::endl;
            // 将收到的数据发送到Kafka
            kafkaProducer.produce(data);
            std::cout << this->name_ << " sent data to Kafka: " << data << std::endl;        });

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


int main() {
    Manager &manager = Manager::getInstance();

    // 配置文件路径和Kafka主题
    std::filesystem::path cPath = std::filesystem::current_path();
    std::filesystem::path configPath = cPath.parent_path() / "kafka_config.txt";
    std::string topic = "topic_0";

    // 状态改变者和消费者对象
    StatusChanger statusChanger;
    Consumer consumer(configPath, topic);

    // 创建N个生产者对象和线程
    int N = 5; // 假设有5个生产者
    std::vector<std::thread> producerThreads;
    for (int i = 0; i < N; ++i) {
        auto* producer = new Producer("Producer" + std::to_string(i+1));
        producerThreads.emplace_back(&Producer::produceData, producer);
    }

    // 创建和启动状态改变者和消费者线程
    std::thread statusChangerThread(&StatusChanger::changeStatus, &statusChanger);
    std::thread consumerThread(&Consumer::consumeData, &consumer, "Consumer");

    // 等待生产者、状态改变者和消费者线程结束
    for (auto& t : producerThreads) {
        t.join();
    }
    statusChangerThread.join();
    consumerThread.join();

    // 睡眠 10 秒
    std::this_thread::sleep_for(std::chrono::seconds(2));


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
