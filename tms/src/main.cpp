/**
 * TMS - Tank Monitoring Subsystem
 * Hardware: ESP32
 * Purpose: Monitor rainwater level using sonar and publish data via MQTT
 * Architecture: FreeRTOS Task-based with FSM for network state management
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Include task headers
#include "task/Config.h"
#include "task/FSM.h"
#include "task/Sensor.h"
#include "task/Network.h"
#include "task/Tasks.h"

// ==================== GLOBAL OBJECTS ====================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ==================== GLOBAL STATE ====================
volatile SystemState currentState = STATE_INITIALIZING;

// ==================== TASK HANDLES ====================
TaskHandle_t sonarTaskHandle = NULL;
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t ledTaskHandle = NULL;

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== TMS - Tank Monitoring Subsystem ===");
    Serial.println("Initializing...");
    
    // Initialize hardware pins
    setupPins();
    
    // Create FreeRTOS Tasks
    createTasks();
    
    // Initial state transition
    handleStateTransition(STATE_CONNECTING_WIFI);
}

// ==================== MAIN LOOP ====================
void loop() {
    // Empty - all work done in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}
