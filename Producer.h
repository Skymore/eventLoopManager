#ifndef PRODUCER_H
#define PRODUCER_H

#include <iostream>
#include <random>
#include <thread>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Manager.h"
#include "Process.h"
#include "SensorReader.h"

class Producer {
public:
    Producer(const std::string &name, double latitude, double longitude);
    Producer(const Producer &producer) = default;
    Producer(Producer &&producer) = default;
    Producer &operator=(const Producer &producer) = default;
    Producer &operator=(Producer &&producer) = default;
    ~Producer();

    void produceData() {
        Process process(producerName);

        for (int i = 1; i <= 10; ++i) {
            rapidjson::Document doc;
            doc.SetObject();
            rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

            // 读取传感器数据
            double temperature = reader->readTemperature();
            double humidity = reader->readHumidity();
            double co2Concentration = reader->readCO2Concentration();

            // 构建JSON文档
            doc.AddMember("id", i, allocator);
            rapidjson::Value nameValue; // 创建一个空的Value对象
            nameValue.SetString(producerName.c_str(), allocator); // 设置字符串值
            doc.AddMember("name", nameValue, allocator); // 添加到文档
            doc.AddMember("temperature", temperature, allocator);
            doc.AddMember("humidity", humidity, allocator);
            doc.AddMember("co2Concentration", co2Concentration, allocator);
            doc.AddMember("latitude", latitude, allocator);
            doc.AddMember("longitude", longitude, allocator);

            // 将文档转换为字符串
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            std::string jsonData = buffer.GetString();

            // 输出JSON数据到控制台（示例用途）
            std::cout << producerName << " sends data: " << jsonData << std::endl;

            // 发送数据到"DataChannel"通道
            process.sendtoChannel("DataChannel", jsonData);

            // 模拟数据产生间隔
            std::this_thread::sleep_for(std::chrono::milliseconds(generateRandomTime()));
        }
    }

private:
    std::string producerName;
    std::unique_ptr<SensorReader> reader;
    double latitude; // 生产者的纬度
    double longitude; // 生产者的经度

    // 生成随机时间，假设在100到500毫秒之间
    static int generateRandomTime() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(500, 1000);
        return dis(gen);
    }
};

Producer::Producer(const std::string &name, double latitude, double longitude) {
    producerName = name;
    this->latitude = latitude;
    this->longitude = longitude;
    reader = std::make_unique<SimulatedSensorReader>();
}

Producer::~Producer() {
    std::cout << "Producer" << producerName <<" is destroyed." << std::endl;
}

#endif //PRODUCER_H