/**
 * WCS Display Functions
 * LCD display management
 */

#ifndef WCS_DISPLAY_H
#define WCS_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Config.h"
#include "FSM.h"
#include "ServoControl.h"

// Global LCD object
extern LiquidCrystal_I2C lcd;

// Timing
extern unsigned long lastLCDUpdate;

// ==================== DISPLAY FUNCTIONS ====================

/**
 * Initialize LCD
 */
void setupLCD() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    Serial.println("LCD initialized");
}

/**
 * Update LCD display with current mode and valve status
 */
void updateLCD() {
    lcd.clear();
    
    // Line 1: Mode
    lcd.setCursor(0, 0);
    lcd.print("Mode: ");
    switch (currentMode) {
        case MODE_UNCONNECTED:
            lcd.print("UNCONN");
            break;
        case MODE_AUTOMATIC:
            lcd.print("AUTO");
            break;
        case MODE_MANUAL:
            lcd.print("MANUAL");
            break;
    }
    
    // Line 2: Valve opening
    lcd.setCursor(0, 1);
    lcd.print("Valve: ");
    lcd.print(currentValvePercentage);
    lcd.print("%");
}

#endif // WCS_DISPLAY_H
