/**
 * TMS Finite State Machine
 * System state management and transitions
 */

#ifndef TMS_FSM_H
#define TMS_FSM_H

#include <Arduino.h>

// ==================== FSM STATES ====================
enum SystemState {
    STATE_INITIALIZING,
    STATE_CONNECTING_WIFI,
    STATE_CONNECTING_MQTT,
    STATE_CONNECTED,
    STATE_NETWORK_ERROR
};

// Global state variable
extern volatile SystemState currentState;

// ==================== FSM FUNCTIONS ====================

/**
 * Handle state transition with logging
 */
void handleStateTransition(SystemState newState) {
    if (currentState != newState) {
        Serial.print("State transition: ");
        Serial.print(currentState);
        Serial.print(" -> ");
        Serial.println(newState);
        
        currentState = newState;
    }
}

#endif // TMS_FSM_H
