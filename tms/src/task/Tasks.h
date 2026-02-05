/**
 * TMS FreeRTOS Tasks
 * Task implementations for sonar reading, MQTT communication, and LED control
 */

#ifndef TMS_TASKS_H
#define TMS_TASKS_H

#include <Arduino.h>
#include "Config.h"
#include "FSM.h"
#include "Sensor.h"
#include "Network.h"

// Task handles
extern TaskHandle_t sonarTaskHandle;
extern TaskHandle_t mqttTaskHandle;
extern TaskHandle_t ledTaskHandle;

// Shared water level variable (protected by atomic operations)
volatile float latestWaterLevel = 0.0;

// ==================== FREERTOS TASKS ====================

/**
 * Sonar Task: Reads water level at configured frequency
 */
void sonarTask(void* parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        if (currentState == STATE_CONNECTED) {
            float waterLevel = readSonarDistance();
            
            if (waterLevel > 0) {
                Serial.print("Water Level: ");
                Serial.print(waterLevel);
                Serial.println(" cm");
                
                // Update shared variable for MQTT task
                latestWaterLevel = waterLevel;
            }
        }
        
        // Wait for next sampling period
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SAMPLING_FREQUENCY_MS));
    }
}

/**
 * MQTT Task: Handles WiFi/MQTT connection and publishes data
 */
void mqttTask(void* parameter) {
    setupWiFi();
    setupMQTT();
    
    for (;;) {
        switch (currentState) {
            case STATE_INITIALIZING:
            case STATE_CONNECTING_WIFI:
                if (WiFi.status() != WL_CONNECTED) {
                    // WiFi connecting...
                    vTaskDelay(pdMS_TO_TICKS(500));
                } else {
                    Serial.println("\nWiFi connected!");
                    Serial.print("IP address: ");
                    Serial.println(WiFi.localIP());
                    handleStateTransition(STATE_CONNECTING_MQTT);
                }
                break;
                
            case STATE_CONNECTING_MQTT:
                if (mqttClient.connect(MQTT_CLIENT_ID)) {
                    Serial.println("MQTT connected!");
                    handleStateTransition(STATE_CONNECTED);
                } else {
                    Serial.print("MQTT connection failed, rc=");
                    Serial.println(mqttClient.state());
                    vTaskDelay(pdMS_TO_TICKS(RECONNECT_DELAY_MS));
                }
                break;
                
            case STATE_CONNECTED:
                if (!mqttClient.connected()) {
                    Serial.println("MQTT disconnected!");
                    handleStateTransition(STATE_NETWORK_ERROR);
                } else {
                    mqttClient.loop();
                    
                    // Publish water level data (plain number format for CUS compatibility)
                    if (latestWaterLevel > 0) {
                        char msg[20];
                        snprintf(msg, sizeof(msg), "%.2f", latestWaterLevel);
                        mqttClient.publish(MQTT_TOPIC_LEVEL, msg);
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(100));
                break;
                
            case STATE_NETWORK_ERROR:
                Serial.println("Attempting to recover from network error...");
                if (WiFi.status() != WL_CONNECTED) {
                    handleStateTransition(STATE_CONNECTING_WIFI);
                } else {
                    handleStateTransition(STATE_CONNECTING_MQTT);
                }
                vTaskDelay(pdMS_TO_TICKS(RECONNECT_DELAY_MS));
                break;
        }
    }
}

/**
 * LED Task: Controls LED indicators based on system state
 * Requirement: Green ON + Red OFF = OK, Red ON + Green OFF = Error
 */
void ledTask(void* parameter) {
    for (;;) {
        switch (currentState) {
            case STATE_CONNECTED:
                // System working correctly: Green ON, Red OFF
                digitalWrite(LED_GREEN_PIN, HIGH);
                digitalWrite(LED_RED_PIN, LOW);
                break;
                
            case STATE_NETWORK_ERROR:
                // Network problems: Red ON, Green OFF
                digitalWrite(LED_GREEN_PIN, LOW);
                digitalWrite(LED_RED_PIN, HIGH);
                break;
                
            default:
                // During initialization/connection: Both OFF
                digitalWrite(LED_GREEN_PIN, LOW);
                digitalWrite(LED_RED_PIN, HIGH);
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * Setup hardware pins
 */
void setupPins() {
    pinMode(SONAR_TRIG_PIN, OUTPUT);
    pinMode(SONAR_ECHO_PIN, INPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    
    Serial.println("Pins configured");
}

/**
 * Create all FreeRTOS tasks
 */
void createTasks() {
    xTaskCreate(
        sonarTask,          // Task function
        "SonarTask",        // Task name
        4096,               // Stack size (bytes)
        NULL,               // Task parameter
        2,                  // Priority
        &sonarTaskHandle    // Task handle
    );
    
    xTaskCreate(
        mqttTask,
        "MQTTTask",
        8192,               // Larger stack for network operations
        NULL,
        1,                  // Lower priority
        &mqttTaskHandle
    );
    
    xTaskCreate(
        ledTask,
        "LEDTask",
        2048,
        NULL,
        1,
        &ledTaskHandle
    );
    
    Serial.println("FreeRTOS tasks created successfully");
}

#endif // TMS_TASKS_H
