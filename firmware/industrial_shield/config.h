
#pragma once
#include <Arduino.h>



// Temperature sensor
const uint8_t TEMP_PIN = 4;

// Gas sensor
const uint8_t GAS_PIN = 34;

// Current sensor (I2C)
const uint8_t INA219_SDA = 21;
const uint8_t INA219_SCL = 22;

// Emergency response
const uint8_t RELAY_PIN = 26;
const uint8_t BUZZER_PIN = 25;

// Status LEDs
const uint8_t LED_SAFE = 16;
const uint8_t LED_WARNING = 17;

// LCD connections
const uint8_t LCD_RS = 13;
const uint8_t LCD_EN = 14;
const uint8_t LCD_D4 = 27;
const uint8_t LCD_D5 = 32;
const uint8_t LCD_D6 = 33;
const uint8_t LCD_D7 = 23;
