/*
  INDUSTRIAL SHIELD — existing sensor/alarm firmware + ThingSpeak Wi-Fi upload

  Based on the team's working firmware supplied by Dolima. Existing local
  sensor checks, buzzer, button, and thresholds are preserved. Telemetry is
  uploaded to ThingSpeak every 16 seconds on a separate ESP32 task so network
  delays do not pause the local buzzer loop.

  Channel fields:
    1 = Gas ADC (raw ESP32 ADC count; not ppm)
    2 = Temperature C (-127 means DS18B20 fault)
    3 = Current mA (-1 means INA219 unavailable)
    4 = Risk Code: -1 sensor fault, 0 safe, 1 warning, 2 critical

  Risk mapping from the existing alarm flags:
    0: no alarms and both temperature/current sensors available
    1: one active hazard flag
    2: two or more simultaneous hazard flags
   -1: no active hazard, but DS18B20 or INA219 is unavailable

  Before upload, copy secrets.example.h to secrets.h and fill it locally.
  Required libraries: ThingSpeak, OneWire, DallasTemperature, Adafruit INA219.
*/

#include <WiFi.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <math.h>
#include "secrets.h"

// secrets.h is local-only and excluded from Git by the repository .gitignore.
const char* WIFI_SSID = SECRET_WIFI_SSID;
const char* WIFI_PASSWORD = SECRET_WIFI_PASSWORD;
const unsigned long CHANNEL_ID = SECRET_CHANNEL_ID;
const char* WRITE_API_KEY = SECRET_WRITE_API_KEY;

// Existing team pin assignments and alarm thresholds — preserved as supplied.
const int TEMP_PIN = 32, GAS_PIN = 34, BUZZ = 25, BTN = 35;
const int PIN_A = 13, PIN_B = 27;

const float TEMP_ON  = 33.0, TEMP_OFF = 31.0;
const int   GAS_ON   = 2100, GAS_OFF  = 1900;
const float CUR_ON   = 500.0, CUR_OFF = 400.0;
const float CUR_JUMP = 150.0;
const int   BUZZ_ON  = HIGH;
const int   BUZZ_OFF = (BUZZ_ON == HIGH) ? LOW : HIGH;

// ThingSpeak free channels require at least 15 seconds between writes.
// Use 16 seconds to stay safely above that limit.
constexpr unsigned long PUBLISH_INTERVAL_MS = 16000;
constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 10000;

WiFiClient cloudClient;
OneWire ow(TEMP_PIN);
DallasTemperature tempSensor(&ow);
Adafruit_INA219 ina;

bool inaOK = false;
bool tempAlarm = false, gasAlarm = false, curAlarm = false, fluctuating = false;
bool muted = false;
float t = 0, mA = 0, busV = 0, lastmA = 0;
bool tempOK = false, haveLast = false;
int gas = 0;

unsigned long lastSensor = 0, lastPress = 0, testUntil = 0;
bool lastPressed = false;

struct TelemetrySnapshot {
  int gasAdc;
  float temperatureC;
  float currentmA;
  int riskCode;
  bool temperatureValid;
  bool currentValid;
};

TelemetrySnapshot cloudSnapshot = {0, -127.0f, -1.0f, -1, false, false};
bool cloudSnapshotReady = false;
portMUX_TYPE cloudSnapshotMux = portMUX_INITIALIZER_UNLOCKED;
unsigned long lastWiFiAttemptMs = 0;

void buzzer(bool on) {
  pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, on ? BUZZ_ON : BUZZ_OFF);
}

bool pingIna(int sda, int scl) {
  Wire.end();
  Wire.begin(sda, scl);
  Wire.setClock(100000);
  Wire.beginTransmission(0x40);
  return Wire.endTransmission() == 0;
}

int calculateRiskCode() {
  const int activeHazards = (tempAlarm ? 1 : 0) + (gasAlarm ? 1 : 0) +
                            (curAlarm ? 1 : 0) + (fluctuating ? 1 : 0);
  if (activeHazards >= 2) return 2; // CRITICAL: multiple active hazard flags
  if (activeHazards == 1) return 1; // WARNING: one active hazard flag
  if (!tempOK || !inaOK) return -1; // SENSOR FAULT / unavailable measurement
  return 0;                         // SAFE
}

void updateCloudSnapshot() {
  TelemetrySnapshot next;
  next.gasAdc = gas;
  next.temperatureC = tempOK ? t : -127.0f;
  next.currentmA = inaOK ? mA : -1.0f;
  next.riskCode = calculateRiskCode();
  next.temperatureValid = tempOK;
  next.currentValid = inaOK;

  portENTER_CRITICAL(&cloudSnapshotMux);
  cloudSnapshot = next;
  cloudSnapshotReady = true;
  portEXIT_CRITICAL(&cloudSnapshotMux);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttemptMs = millis();
  Serial.printf("Connecting to phone hotspot: %s\n", WIFI_SSID);
}

void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - lastWiFiAttemptMs < WIFI_RETRY_INTERVAL_MS) return;

  lastWiFiAttemptMs = millis();
  Serial.println("Wi-Fi disconnected; retrying...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void cloudUploadTask(void* parameter) {
  unsigned long lastPublishMs = 0;
  bool hasAttempted = false;

  for (;;) {
    maintainWiFi();

    TelemetrySnapshot data;
    bool ready;
    portENTER_CRITICAL(&cloudSnapshotMux);
    data = cloudSnapshot;
    ready = cloudSnapshotReady;
    portEXIT_CRITICAL(&cloudSnapshotMux);

    const unsigned long now = millis();
    const bool intervalReady = !hasAttempted ||
                               (now - lastPublishMs >= PUBLISH_INTERVAL_MS);

    if (WiFi.status() == WL_CONNECTED && ready && intervalReady) {
      hasAttempted = true;
      lastPublishMs = now;

      ThingSpeak.setField(1, data.gasAdc);
      ThingSpeak.setField(2, data.temperatureC);
      ThingSpeak.setField(3, data.currentmA);
      ThingSpeak.setField(4, data.riskCode);

      const int response = ThingSpeak.writeFields(CHANNEL_ID, WRITE_API_KEY);
      if (response == 200) {
        Serial.printf("ThingSpeak upload OK | gas=%d ADC, temp=%.2f C, current=%.2f mA, risk=%d\n",
                      data.gasAdc, data.temperatureC, data.currentmA, data.riskCode);
      } else {
        Serial.printf("ThingSpeak upload failed | response=%d\n", response);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup() {
  Serial.begin(115200);
  buzzer(false);
  pinMode(BTN, INPUT);
  tempSensor.begin();
  tempSensor.setWaitForConversion(false);
  tempSensor.requestTemperatures();

  Serial.println();
  Serial.println("===== INDUSTRIAL HAZARD DETECTION SYSTEM =====");
  if (pingIna(PIN_A, PIN_B) || pingIna(PIN_B, PIN_A)) inaOK = ina.begin(&Wire);
  Serial.print("Current sensor (INA219): ");
  Serial.println(inaOK ? "CONNECTED" : "NOT CONNECTED (current checks skipped)");
  Serial.println("Button: press = silence alarm, or test beep when no alarm");

  ThingSpeak.begin(cloudClient);
  connectWiFi();

  // Cloud/network delays stay off the main sensor-and-buzzer loop.
  xTaskCreatePinnedToCore(cloudUploadTask, "ThingSpeakUpload", 8192,
                          nullptr, 1, nullptr, 0);
  delay(1500);
}

void loop() {
  const unsigned long now = millis();

  const bool pressed = (digitalRead(BTN) == LOW);
  if (pressed && !lastPressed && now - lastPress > 250) {
    lastPress = now;
    if (tempAlarm || gasAlarm || curAlarm || fluctuating) {
      muted = true;
      Serial.println(">>> BUTTON: alarm silenced (MUTED)");
    } else {
      testUntil = now + 500;
      Serial.println(">>> BUTTON: buzzer test beep");
    }
  }
  lastPressed = pressed;

  if (now - lastSensor >= 1000) {
    lastSensor = now;

    t = tempSensor.getTempCByIndex(0);
    tempOK = (t != DEVICE_DISCONNECTED_C) && !isnan(t);
    tempSensor.requestTemperatures();
    gas = analogRead(GAS_PIN);

    if (tempOK) {
      if (!tempAlarm && t > TEMP_ON) tempAlarm = true;
      if (tempAlarm && t <= TEMP_OFF) tempAlarm = false;
    }

    if (!gasAlarm && gas > GAS_ON) gasAlarm = true;
    if (gasAlarm && gas <= GAS_OFF) gasAlarm = false;

    fluctuating = false;
    if (inaOK) {
      mA = ina.getCurrent_mA();
      busV = ina.getBusVoltage_V();
      if (!curAlarm && mA > CUR_ON) curAlarm = true;
      if (curAlarm && mA <= CUR_OFF) curAlarm = false;
      if (haveLast && fabs(mA - lastmA) > CUR_JUMP) fluctuating = true;
      lastmA = mA;
      haveLast = true;
    }

    const bool alarm = tempAlarm || gasAlarm || curAlarm || fluctuating;
    if (!alarm) muted = false;

    Serial.println("----------------------------------------");
    Serial.print("Temperature : ");
    if (tempOK) { Serial.print(t, 1); Serial.print(" C  -> "); }
    else Serial.print("--  -> ");
    Serial.println(!tempOK ? "SENSOR ERROR" : (tempAlarm ? "HIGH TEMPERATURE" : "NORMAL"));

    Serial.print("Gas level   : ");
    Serial.print(gas);
    Serial.print("  -> ");
    Serial.println(gasAlarm ? "GAS DETECTED" : "NORMAL");

    Serial.print("Current     : ");
    if (inaOK) {
      Serial.print(mA, 1);
      Serial.print(" mA, ");
      Serial.print(busV, 2);
      Serial.print(" V  -> ");
      Serial.println(curAlarm ? "OVER-CURRENT" : (fluctuating ? "CURRENT FLUCTUATING" : "STABLE"));
    } else {
      Serial.println("--  -> SENSOR NOT CONNECTED");
    }

    if (alarm && muted) Serial.println("STATUS      : ALARM (buzzer MUTED by button)");
    else if (alarm)     Serial.println("STATUS      : *** ALARM ***   BUZZER: ON");
    else                Serial.println("STATUS      : ALL NORMAL      BUZZER: OFF");

    updateCloudSnapshot();
  }

  const bool alarmNow = tempAlarm || gasAlarm || curAlarm || fluctuating;
  buzzer((alarmNow && !muted) || now < testUntil);
  delay(20);
}
