
#pragma once

struct SensorData {
    float temperature;
    int gasLevel;
    float voltage;
    float current;
    float power;
};

SensorData getSimulatedSensorData() {
    SensorData data;

    data.temperature = 32.5;
    data.gasLevel = 180;
    data.voltage = 12.0;
    data.current = 0.8;
    data.power = data.voltage * data.current;

    return data;
}
