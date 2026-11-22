
#include <Arduino.h>

#include "config.h"
#include "sensor_data.h"
#include "temperature_sensor.h"
#include "gas_sensor.h"
#include "power_monitor.h"
#include "risk_assessment.h"
#include "emergency_response.h"
#include "lcd_display.h"
#include "wifi_manager.h"
#include "backend_client.h"

bool inaAvailable = false;

unsigned long lastReadingTime = 0;
unsigned long lastBackendTime = 0;

SensorData readings;

void setup() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("================================");
    Serial.println("       INDUSTRIAL SHIELD");
    Serial.println("================================");

    initTemperatureSensor();
    initGasSensor();
    setupEmergencyResponse();
    initLCD();

    inaAvailable = initPowerMonitor();

    Serial.print("INA219: ");
    Serial.println(inaAvailable ? "Connected" : "Not detected");

    connectWiFi();

    Serial.println("Initialization complete");
}

void loop() {
    unsigned long now = millis();

    if (now - lastReadingTime >= 1000) {
        lastReadingTime = now;

        readings.temperature = readTemperature();
        readings.gasLevel = readGasLevel();

        readings.voltage = 0;
        readings.current = 0;
        readings.power = 0;

        if (inaAvailable) {
            readings.voltage = readBusVoltage();
            readings.current = readCurrent();
            readings.power = readPower();
        }

        RiskLevel risk = calculateRisk(readings);

        updateEmergencyResponse(risk);

        updateLCD(
            readings.temperature,
            readings.gasLevel,
            risk
        );

        Serial.println("------------------------------");

        Serial.print("Temperature: ");
        Serial.println(readings.temperature);

        Serial.print("Gas: ");
        Serial.println(readings.gasLevel);

        if (inaAvailable) {
            Serial.print("Voltage: ");
            Serial.println(readings.voltage);

            Serial.print("Current: ");
            Serial.println(readings.current);

            Serial.print("Power: ");
            Serial.println(readings.power);
        }

        Serial.print("Risk: ");
        Serial.println(getRiskLabel(risk));

        Serial.println("------------------------------");
    }

    if (now - lastBackendTime >= 2000) {
        lastBackendTime = now;

        // Backend model rejects the -999 sensor-disconnected
        // value, so do not submit invalid temperature readings.
        if (readings.temperature > -100) {
            bool buzzerOn =
                digitalRead(BUZZER_PIN) == HIGH;

            sendSensorData(
                readings.gasLevel,
                readings.temperature,
                false,
                buzzerOn
            );
        } else {
            Serial.println(
                "Backend skipped: temperature sensor error"
            );
        }
    }

    delay(10);
}
