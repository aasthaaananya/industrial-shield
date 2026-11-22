
#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

void initTemperatureSensor() {
    tempSensor.begin();
}

float readTemperature() {
    tempSensor.requestTemperatures();

    float temperature = tempSensor.getTempCByIndex(0);

    if (temperature == DEVICE_DISCONNECTED_C) {
        return -999.0;
    }

    return temperature;
}
