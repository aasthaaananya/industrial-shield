
#pragma once

#include <Arduino.h>
#include "config.h"
#include "risk_assessment.h"

// Relay activation is disabled until hardware polarity
// and its emergency function are verified.
#define ENABLE_RELAY_TRIP 0

// Change these only after checking the relay module.
#define RELAY_TRIP_LEVEL HIGH
#define RELAY_SAFE_LEVEL LOW

static bool alarmAcknowledged = false;
static bool previousRiskAvailable = false;
static RiskLevel previousRisk = SAFE;

static unsigned long lastBlinkTime = 0;
static unsigned long lastBuzzerTime = 0;

static bool ledState = false;
static bool buzzerState = false;

inline void setupEmergencyResponse() {
    pinMode(LED_SAFE, OUTPUT);
    pinMode(LED_WARNING, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    digitalWrite(LED_SAFE, HIGH);
    digitalWrite(LED_WARNING, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN, RELAY_SAFE_LEVEL);

    Serial.println("Emergency Response System Ready");
}

inline void acknowledgeEmergency() {
    alarmAcknowledged = true;
    buzzerState = false;
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("Emergency Alarm Acknowledged");
}

inline bool resetEmergencyResponse(RiskLevel currentRisk) {
    if (currentRisk != SAFE) {
        Serial.println("Reset denied: Risk is still active");
        return false;
    }

    alarmAcknowledged = false;
    buzzerState = false;
    ledState = false;

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_WARNING, LOW);
    digitalWrite(LED_SAFE, HIGH);
    digitalWrite(RELAY_PIN, RELAY_SAFE_LEVEL);

    Serial.println("Emergency System Reset");
    return true;
}

inline void updateEmergencyResponse(RiskLevel risk) {

    unsigned long currentTime = millis();

    if (previousRiskAvailable && risk != previousRisk) {
        alarmAcknowledged = false;
    }

    previousRisk = risk;
    previousRiskAvailable = true;

    if (risk == SAFE) {
        digitalWrite(LED_SAFE, HIGH);
        digitalWrite(LED_WARNING, LOW);
        digitalWrite(BUZZER_PIN, LOW);

        buzzerState = false;
        ledState = false;

#if ENABLE_RELAY_TRIP
        digitalWrite(RELAY_PIN, RELAY_SAFE_LEVEL);
#endif

        return;
    }

    digitalWrite(LED_SAFE, LOW);

    unsigned long blinkInterval = 1000;
    unsigned long buzzerInterval = 0;

    if (risk == WARNING) {
        blinkInterval = 1000;
    }
    else if (risk == DANGER) {
        blinkInterval = 400;
        buzzerInterval = 600;
    }
    else if (risk == CRITICAL) {
        blinkInterval = 150;
        buzzerInterval = 250;
    }

    if (currentTime - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = currentTime;
        ledState = !ledState;
        digitalWrite(LED_WARNING, ledState);
    }

    if (buzzerInterval > 0 && !alarmAcknowledged) {
        if (currentTime - lastBuzzerTime >= buzzerInterval) {
            lastBuzzerTime = currentTime;
            buzzerState = !buzzerState;
            digitalWrite(BUZZER_PIN, buzzerState);
        }
    }
    else {
        buzzerState = false;
        digitalWrite(BUZZER_PIN, LOW);
    }

#if ENABLE_RELAY_TRIP
    digitalWrite(
        RELAY_PIN,
        risk == CRITICAL ? RELAY_TRIP_LEVEL : RELAY_SAFE_LEVEL
    );
#endif
}
