
#pragma once

#include <Arduino.h>
#include "config.h"

void initGasSensor() {
    pinMode(GAS_PIN, INPUT);
}

int readGasLevel() {
    return analogRead(GAS_PIN);
}
