/**
 * WCS Finite State Machine
 * System mode management and transitions
 */

#ifndef WCS_FSM_H
#define WCS_FSM_H

#include <Arduino.h>
#include "Config.h"

// ==================== FSM STATES ====================
enum SystemMode {
    MODE_UNCONNECTED,   // No connection with CUS
    MODE_AUTOMATIC,     // CUS controls valve
    MODE_MANUAL         // Local control via potentiometer
};

// Global mode variables
extern SystemMode currentMode;
extern SystemMode previousMode;
extern unsigned long lastCUSMessageTime;

// ==================== FSM FUNCTIONS ====================

/**
 * Handle mode transition with logging and LCD update
 */
void handleModeTransition(SystemMode newMode);  // Forward declaration

/**
 * Update FSM state based on timeout and current mode
 */
void updateFSM();  // Forward declaration

#endif // WCS_FSM_H
