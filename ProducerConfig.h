#ifndef EVENTLOOPMANAGER_PRODUCERCONFIG_H
#define EVENTLOOPMANAGER_PRODUCERCONFIG_H

#include <fstream>
#include <vector>
#include "rapidjson/document.h"


class ProducerConfig {
public:
    std::string name;
    double latitude;
    double longitude;

    static std::vector<ProducerConfig> loadFromJson(const std::string& filePath) {
        std::vector<ProducerConfig> configs;
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        rapidjson::Document doc;
        if (doc.Parse(content.c_str()).HasParseError()) {
            std::cerr << "Error parsing JSON." << std::endl;
            return configs; // 或者是其他错误处理方式
        }
        const auto& producers = doc["producers"].GetArray();

        for (const auto& producer : producers) {
            ProducerConfig config;
            config.name = producer["name"].GetString();
            config.latitude = producer["latitude"].GetDouble();
            config.longitude = producer["longitude"].GetDouble();
            configs.push_back(config);
        }

        return configs;
    }
};


#endif //EVENTLOOPMANAGER_PRODUCERCONFIG_H
