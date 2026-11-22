
#pragma once

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "config.h"
#include "risk_assessment.h"

LiquidCrystal lcd(
    LCD_RS, LCD_EN,
    LCD_D4, LCD_D5, LCD_D6, LCD_D7
);

void initLCD() {
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INDUSTRIAL SHLD");
    lcd.setCursor(0, 1);
    lcd.print("Starting...");
}

void updateLCD(float temperature, int gasLevel, RiskLevel risk) {
    lcd.setCursor(0, 0);
    lcd.print("T:");

    if (temperature <= -100) {
        lcd.print("ERR ");
    } else {
        lcd.print(temperature, 1);
        lcd.print("C ");
    }

    lcd.print("G:");
    lcd.print(gasLevel);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print("Status: ");
    lcd.print(getRiskLabel(risk));
    lcd.print("       ");
}
