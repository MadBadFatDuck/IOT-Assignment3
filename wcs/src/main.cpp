/**
 * WCS - Water Channel Subsystem
 * Hardware: Arduino UNO
 * Purpose: Control water valve and provide local operator interface
 * Architecture: FSM-based with no blocking delays
 */

#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// Include task headers
#include "task/Config.h"
#include "task/FSM.h"
#include "task/Input.h"
#include "task/ServoControl.h"
#include "task/Display.h"
#include "task/SerialComm.h"
#include "task/Logic.h"

// ==================== GLOBAL OBJECTS ====================
Servo valveServo;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// ==================== GLOBAL STATE VARIABLES ====================
SystemMode currentMode = MODE_UNCONNECTED;
SystemMode previousMode = MODE_UNCONNECTED;

int currentValvePercentage = 0;  // 0-100%
int targetValvePercentage = 0;   // Target from CUS or potentiometer

// Button state
bool lastButtonState = HIGH;
bool stableButtonState = HIGH;
unsigned long lastButtonDebounceTime = 0;

// Potentiometer state
int lastPotValue = -1;
unsigned long ignorePotUntil = 0;

// Timing variables
unsigned long lastLCDUpdate = 0;
unsigned long lastSerialUpdate = 0;
unsigned long lastCUSMessageTime = 0;

// ==================== SETUP ====================
void setup() {
    setupSerial();
    Serial.println("\n=== WCS - Water Channel Subsystem ===");
    Serial.println("Initializing...");
    
    setupPins();
    setupServo();
    setupLCD();
    
    // Initial display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WCS Ready");
    lcd.setCursor(0, 1);
    lcd.print("Mode: UNCONN");
    
    delay(1000);
    
    Serial.println("Initialization complete");
    Serial.println("Waiting for CUS connection...");
}

// ==================== MAIN LOOP ====================
void loop() {
    // Non-blocking FSM update
    updateFSM();
    
    // Handle button input
    handleButtonPress();
    
    // Handle serial communication
    handleSerialInput();
    
    // Update LCD periodically
    if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL_MS) {
        updateLCD();
        lastLCDUpdate = millis();
    }
    
    // Send status to CUS periodically
    if (millis() - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL_MS) {
        sendStatusToSerial();
        lastSerialUpdate = millis();
    }
}
