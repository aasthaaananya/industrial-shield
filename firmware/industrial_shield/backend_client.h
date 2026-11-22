
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* BACKEND_URL =
     "http://192.168.1.4:8000/api/sensors/readings";

void sendSensorData(
    float gasValue,
    float temperature,
    bool relayOn,
    bool buzzerOn
) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Backend skipped: Wi-Fi disconnected");
        return;
    }

    HTTPClient http;

    http.begin(BACKEND_URL);
    http.setTimeout(3000);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;

    doc["gas_value"] = gasValue;
    doc["temperature"] = temperature;
    doc["relay_on"] = relayOn;
    doc["buzzer_on"] = buzzerOn;

    String requestBody;
    serializeJson(doc, requestBody);

    int responseCode = http.POST(requestBody);

    Serial.print("Backend response: ");
    Serial.println(responseCode);

    if (responseCode > 0) {
        Serial.println(http.getString());
    }

    http.end();
}
