/**
 * WCS Serial Communication
 * Serial communication with CUS (JSON protocol)
 */

#ifndef WCS_SERIAL_COMM_H
#define WCS_SERIAL_COMM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FSM.h"
#include "ServoControl.h"

// Timing
extern unsigned long lastSerialUpdate;

// ==================== SERIAL COMMUNICATION FUNCTIONS ====================

/**
 * Initialize serial communication
 */
void setupSerial() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {
        ; // Wait for serial port to connect (needed for native USB)
    }
}

/**
 * Process incoming JSON command from CUS
 * @param command JSON command string
 */
void processSerialCommand(const String& command) {
    // Parse JSON command from CUS
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, command);
    
    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }
    
    const char* cmd = doc["cmd"];
    
    if (strcmp(cmd, "set_valve") == 0) {
        // Command to set valve percentage
        int value = doc["value"];
        Serial.print("Received valve command: ");
        Serial.println(value);
        
        if (currentMode == MODE_AUTOMATIC) {
            targetValvePercentage = value;
        } else {
            Serial.println("Ignored - not in AUTOMATIC mode");
        }
        
    } else if (strcmp(cmd, "set_mode") == 0) {
        // Command to change mode
        const char* mode = doc["value"];
        Serial.print("Received mode command: ");
        Serial.println(mode);
        
        if (strcmp(mode, "AUTOMATIC") == 0) {
            handleModeTransition(MODE_AUTOMATIC);
        } else if (strcmp(mode, "MANUAL") == 0) {
            handleModeTransition(MODE_MANUAL);
        }
    }
}

/**
 * Handle incoming serial data
 */
void handleSerialInput() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            processSerialCommand(command);
            lastCUSMessageTime = millis();  // Reset timeout
            
            // If we were unconnected, transition to automatic
            if (currentMode == MODE_UNCONNECTED) {
                handleModeTransition(MODE_AUTOMATIC);
            }
        }
    }
}

/**
 * Send current status to CUS in JSON format
 */
void sendStatusToSerial() {
    // Send current status to CUS in JSON format
    StaticJsonDocument<200> doc;
    
    switch (currentMode) {
        case MODE_UNCONNECTED:
            doc["mode"] = "UNCONNECTED";
            break;
        case MODE_AUTOMATIC:
            doc["mode"] = "AUTOMATIC";
            break;
        case MODE_MANUAL:
            doc["mode"] = "MANUAL";
            break;
    }
    
    doc["valve"] = currentValvePercentage;
    
    serializeJson(doc, Serial);
    Serial.println();  // End of JSON message
}

#endif // WCS_SERIAL_COMM_H
