
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#include "config.h"

Adafruit_INA219 ina219;

bool initPowerMonitor() {
    Wire.begin(INA219_SDA, INA219_SCL);
    return ina219.begin();
}

float readBusVoltage() {
    return ina219.getBusVoltage_V();
}

float readCurrent() {
    return ina219.getCurrent_mA();
}

float readPower() {
    return ina219.getPower_mW();
}
