
#pragma once

#include <Arduino.h>
#include "config.h"
#include "risk_assessment.h"

bool alarmAcknowledged = false;
bool lastButtonState = HIGH;
bool buttonHandled = false;

unsigned long lastButtonChange = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastBuzzerTime = 0;
unsigned long testUntil = 0;

bool warningLedState = false;
bool buzzerState = false;

void setupEmergencyResponse() {
    pinMode(BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_SAFE, OUTPUT);
    pinMode(LED_WARNING, OUTPUT);

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_SAFE, LOW);
    digitalWrite(LED_WARNING, LOW);

    Serial.println("Emergency Response Ready");
}

void updateEmergencyResponse(RiskLevel risk) {
    unsigned long now = millis();
    bool buttonState = digitalRead(BUTTON_PIN);

    if (buttonState != lastButtonState) {
        lastButtonChange = now;
        lastButtonState = buttonState;
    }

    if (now - lastButtonChange >= 40) {
        if (buttonState == LOW && !buttonHandled) {
            buttonHandled = true;

            if (risk != SAFE) {
                alarmAcknowledged = true;
                buzzerState = false;
                digitalWrite(BUZZER_PIN, LOW);
                Serial.println("Alarm silenced");
            } else {
                testUntil = now + 500;
                Serial.println("Buzzer test");
            }
        }

        if (buttonState == HIGH) {
            buttonHandled = false;
        }
    }

    if (risk == SAFE) {
        alarmAcknowledged = false;
        warningLedState = false;
        buzzerState = false;

        digitalWrite(LED_SAFE, HIGH);
        digitalWrite(LED_WARNING, LOW);
        digitalWrite(
            BUZZER_PIN,
            now < testUntil ? HIGH : LOW
        );

        return;
    }

    digitalWrite(LED_SAFE, LOW);

    unsigned long blinkInterval =
        (risk == CRITICAL) ? 150 : 700;

    if (now - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = now;
        warningLedState = !warningLedState;
        digitalWrite(LED_WARNING, warningLedState);
    }

    if (risk == CRITICAL && !alarmAcknowledged) {
        if (now - lastBuzzerTime >= 250) {
            lastBuzzerTime = now;
            buzzerState = !buzzerState;
        }

        digitalWrite(BUZZER_PIN, buzzerState);
    } else {
        buzzerState = false;
        digitalWrite(BUZZER_PIN, LOW);
    }
}
