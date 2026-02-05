/**
 * WCS Configuration
 * All system configuration constants and pin definitions
 */

#ifndef WCS_CONFIG_H
#define WCS_CONFIG_H

// ==================== HARDWARE PIN CONFIGURATION ====================
const int SERVO_PIN = 9;
const int POTENTIOMETER_PIN = A0;
const int BUTTON_PIN = 2;

// ==================== LCD CONFIGURATION ====================
const int LCD_ADDRESS = 0x27;
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// ==================== TIMING CONFIGURATION ====================
const unsigned long BUTTON_DEBOUNCE_MS = 50;
const unsigned long LCD_UPDATE_INTERVAL_MS = 500;
const unsigned long SERIAL_UPDATE_INTERVAL_MS = 500;
const unsigned long CUS_TIMEOUT_MS = 5000;  // 5 seconds without CUS message -> UNCONNECTED

// ==================== VALVE CONFIGURATION ====================
const int VALVE_MIN_ANGLE = 0;    // 0% = Closed
const int VALVE_MAX_ANGLE = 90;   // 100% = Fully open

// ==================== SERIAL CONFIGURATION ====================
const unsigned long SERIAL_BAUD = 9600;

#endif // WCS_CONFIG_H
