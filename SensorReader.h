#ifndef EVENTLOOPMANAGER_SENSORREADER_H
#define EVENTLOOPMANAGER_SENSORREADER_H


#include <random>

class SensorReader {
public:
    virtual float readTemperature() = 0;
    virtual float readHumidity() = 0;
    virtual float readCO2Concentration() = 0;
    ~SensorReader() = default;
};

class SimulatedSensorReader : public SensorReader {
public:
    float readTemperature() override {
        return generateRandom(0.0, 30.0);
    }

    float readHumidity() override {
        return generateRandom(0.0, 90.0);
    }

    float readCO2Concentration() override {
        return generateRandom(400.0, 1000.0);
    }

private:
    float generateRandom(float min, float max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        return dis(gen);
    }
};



#endif //EVENTLOOPMANAGER_SENSORREADER_H
