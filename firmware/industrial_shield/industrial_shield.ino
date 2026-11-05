
#include "config.h"
#include "risk_assessment.h"
#include <Arduino.h>

void setup() {
    Serial.begin(115200);

    Serial.println("================================");
    Serial.println("       INDUSTRIAL SHIELD");
    Serial.println("================================");
    Serial.println("System Initializing...");
    Serial.println("ESP32 Ready.");
}

void loop() {
    SensorData readings = getSimulatedSensorData();

    RiskLevel risk = calculateRisk(readings);

    Serial.println("-------------------------------");

    Serial.print("Temperature: ");
    Serial.print(readings.temperature);
    Serial.println(" C");

    Serial.print("Gas Level: ");
    Serial.println(readings.gasLevel);

    Serial.print("Voltage: ");
    Serial.print(readings.voltage);
    Serial.println(" V");

    Serial.print("Current: ");
    Serial.print(readings.current);
    Serial.println(" A");

    Serial.print("Power: ");
    Serial.print(readings.power);
    Serial.println(" W");

    Serial.print("Risk Status: ");
    Serial.println(getRiskLabel(risk));

    delay(2000);
}
