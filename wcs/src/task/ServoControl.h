/**
 * WCS Servo Control
 * Valve control and servo management
 */

#ifndef WCS_SERVO_CONTROL_H
#define WCS_SERVO_CONTROL_H

#include <Arduino.h>
#include <Servo.h>
#include "Config.h"

// Global servo object
extern Servo valveServo;

// Global valve state
extern int currentValvePercentage;
extern int targetValvePercentage;

// ==================== SERVO CONTROL FUNCTIONS ====================

/**
 * Convert percentage to servo angle
 * @param percentage Valve opening percentage (0-100)
 * @return Servo angle (VALVE_MIN_ANGLE to VALVE_MAX_ANGLE)
 */
int percentageToAngle(int percentage) {
    return map(percentage, 0, 100, VALVE_MIN_ANGLE, VALVE_MAX_ANGLE);
}

/**
 * Set valve to specified percentage
 * @param percentage Valve opening percentage (0-100)
 */
void setValvePercentage(int percentage) {
    percentage = constrain(percentage, 0, 100);
    int angle = percentageToAngle(percentage);
    
    valveServo.write(angle);
    currentValvePercentage = percentage;
}

/**
 * Initialize servo
 */
void setupServo() {
    valveServo.attach(SERVO_PIN);
    setValvePercentage(0);  // Start with valve closed
    Serial.println("Servo initialized (valve closed)");
}

#endif // WCS_SERVO_CONTROL_H
