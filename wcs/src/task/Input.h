/**
 * WCS Input Handling
 * Button debouncing and potentiometer reading
 */

#ifndef WCS_INPUT_H
#define WCS_INPUT_H

#include <Arduino.h>
#include "Config.h"
#include "FSM.h"

// Global input state
extern bool lastButtonState;
extern bool stableButtonState;
extern unsigned long lastButtonDebounceTime;

// ==================== INPUT FUNCTIONS ====================

/**
 * Handle button press with debouncing
 * Toggles between AUTOMATIC and MANUAL modes
 */
void handleButtonPress();  // Forward declaration

/**
 * Read potentiometer value and convert to percentage
 * @return Valve percentage (0-100)
 */
extern int lastPotValue;
extern unsigned long ignorePotUntil;

/**
 * Check if potentiometer value has changed significantly (hysteresis)
 * @return true if changed beyond threshold
 */
bool hasPotentiometerChanged() {
    int currentVal = analogRead(POTENTIOMETER_PIN);
    // Hysteresis threshold (approx 2% of 1023)
    const int THRESHOLD = 20; 
    
    if (abs(currentVal - lastPotValue) > THRESHOLD) {
        lastPotValue = currentVal;
        return true;
    }
    return false;
}

/**
 * Read potentiometer value and convert to percentage
 * @return Valve percentage (0-100)
 */
int readPotentiometerPercentage() {
    int rawValue = analogRead(POTENTIOMETER_PIN);
    // Map 0-1023 to 0-100%
    int percentage = map(rawValue, 0, 1023, 0, 100);
    return constrain(percentage, 0, 100);
}

#endif // WCS_INPUT_H
