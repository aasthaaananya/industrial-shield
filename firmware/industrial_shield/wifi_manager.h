
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

void connectWiFi() {
    Serial.println("Connecting to Wi-Fi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Wi-Fi connected");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Wi-Fi connection failed");
    }
}
