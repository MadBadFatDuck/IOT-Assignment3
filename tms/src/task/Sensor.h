/**
 * TMS Sensor Functions
 * Sonar sensor reading and distance calculation
 */

#ifndef TMS_SENSOR_H
#define TMS_SENSOR_H

#include <Arduino.h>
#include "Config.h"

// ==================== SENSOR FUNCTIONS ====================

/**
 * Read distance from ultrasonic sonar sensor
 * @return Distance in cm, or 0.0 if out of range
 */
float readSonarDistance() {
    // Send ultrasonic pulse
    digitalWrite(SONAR_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(SONAR_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(SONAR_TRIG_PIN, LOW);
    
    // Measure echo duration
    long duration = pulseIn(SONAR_ECHO_PIN, HIGH, 30000); // 30ms timeout
    
    // Calculate distance in cm
    // Speed of sound: 343 m/s = 0.0343 cm/μs
    // Distance = (duration / 2) * 0.0343
    float distance = (duration * 0.0343) / 2.0;
    
    // Return 0 if out of range
    if (duration == 0 || distance > 400) {
        return 0.0;
    }
    
    return distance;
}

#endif // TMS_SENSOR_H
