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
        // Removed debug prints to avoid breaking JSON protocol
        // Serial.print("Mode transition: "); ...
        
        previousMode = currentMode;
        currentMode = newMode;
        
        // Force LCD update on mode change
        updateLCD();
        
        // Force immediate Serial update to notify CUS
        sendStatusToSerial();
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
            // In manual mode, allow control from both Potentiometer and Serial (Hybrid)
            // Priority given to last interaction
            
            // Check if user moved potentiometer (and not ignored due to recent serial command)
            if (millis() > ignorePotUntil && hasPotentiometerChanged()) {
                targetValvePercentage = readPotentiometerPercentage();
            }
            // else: keep targetValvePercentage as is (it might have been set by Serial)
            
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
    bool reading = digitalRead(BUTTON_PIN);
    
    // Check for reading change (noise or press)
    if (reading != lastButtonState) {
        lastButtonDebounceTime = millis();
    }
    
    if ((millis() - lastButtonDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // Reading has been stable for long enough
        
        // If the button state has changed
        if (reading != stableButtonState) {
            stableButtonState = reading;
            
            // Only act on falling edge (HIGH -> LOW)
            if (stableButtonState == LOW) {
                // Serial.println("Button pressed - toggling mode"); // REMOVED
                
                // Toggle between AUTOMATIC and MANUAL (ignore if UNCONNECTED)
                if (currentMode == MODE_AUTOMATIC) {
                    handleModeTransition(MODE_MANUAL);
                } else if (currentMode == MODE_MANUAL) {
                    handleModeTransition(MODE_AUTOMATIC);
                } else {
                    // Serial.println("Cannot toggle mode - UNCONNECTED"); // REMOVED
                }
            }
        }
    }
    
    lastButtonState = reading;
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
