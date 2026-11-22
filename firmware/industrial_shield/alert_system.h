
#pragma once

#include <Arduino.h>
#include "config.h"
#include "risk_assessment.h"

void initAlertSystem() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_SAFE, OUTPUT);
    pinMode(LED_WARNING, OUTPUT);

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_SAFE, LOW);
    digitalWrite(LED_WARNING, LOW);
}

void updateAlerts(RiskLevel risk) {

    digitalWrite(LED_SAFE, risk == SAFE ? HIGH : LOW);

    digitalWrite(LED_WARNING,
                 risk == WARNING || risk == CRITICAL
                 ? HIGH : LOW);

    digitalWrite(BUZZER_PIN,
                 risk == CRITICAL ? HIGH : LOW);
}
