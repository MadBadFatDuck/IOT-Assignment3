/**
 * WCS Logic
 * FSM update and button handling implementations
 */

#ifndef WCS_LOGIC_H
#define WCS_LOGIC_H

#include <Arduino.h>
#include "Config.h"
#include "FSM.h"
#include "Input.h"
#include "ServoControl.h"
#include "Display.h"

// ==================== FSM IMPLEMENTATION ====================

/**
 * Handle mode transition with logging and LCD update
 */
void handleModeTransition(SystemMode newMode) {
    if (currentMode != newMode) {
        Serial.print("Mode transition: ");
        Serial.print(currentMode);
        Serial.print(" -> ");
        Serial.println(newMode);
        
        previousMode = currentMode;
        currentMode = newMode;
        
        // Force LCD update on mode change
        updateLCD();
    }
}

/**
 * Update FSM state based on timeout and current mode
 */
void updateFSM() {
    // Check for CUS timeout
    if (currentMode != MODE_UNCONNECTED) {
        if (millis() - lastCUSMessageTime > CUS_TIMEOUT_MS) {
            handleModeTransition(MODE_UNCONNECTED);
        }
    }
    
    // Execute state-specific logic
    switch (currentMode) {
        case MODE_UNCONNECTED:
            // In unconnected mode, keep valve at last known position
            // Wait for CUS connection
            break;
            
        case MODE_AUTOMATIC:
            // In automatic mode, valve is controlled by CUS via serial
            // Apply target valve percentage received from CUS
            if (currentValvePercentage != targetValvePercentage) {
                setValvePercentage(targetValvePercentage);
            }
            break;
            
        case MODE_MANUAL:
            // In manual mode, read potentiometer and control valve
            targetValvePercentage = readPotentiometerPercentage();
            if (currentValvePercentage != targetValvePercentage) {
                setValvePercentage(targetValvePercentage);
            }
            break;
    }
}

// ==================== INPUT IMPLEMENTATION ====================

/**
 * Handle button press with debouncing
 * Toggles between AUTOMATIC and MANUAL modes
 */
void handleButtonPress() {
    bool buttonState = digitalRead(BUTTON_PIN);
    
    // Check for button state change with debouncing
    if (buttonState != lastButtonState) {
        lastButtonDebounceTime = millis();
    }
    
    if ((millis() - lastButtonDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // Stable button state
        if (buttonState == LOW && lastButtonState == HIGH) {
            // Button pressed (falling edge)
            Serial.println("Button pressed - toggling mode");
            
            // Toggle between AUTOMATIC and MANUAL (ignore if UNCONNECTED)
            if (currentMode == MODE_AUTOMATIC) {
                handleModeTransition(MODE_MANUAL);
            } else if (currentMode == MODE_MANUAL) {
                handleModeTransition(MODE_AUTOMATIC);
            } else {
                Serial.println("Cannot toggle mode - UNCONNECTED");
            }
        }
    }
    
    lastButtonState = buttonState;
}

// ==================== SETUP FUNCTIONS ====================

/**
 * Setup hardware pins
 */
void setupPins() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(POTENTIOMETER_PIN, INPUT);
    Serial.println("Pins configured");
}

#endif // WCS_LOGIC_H
