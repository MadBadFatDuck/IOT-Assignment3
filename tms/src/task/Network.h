/**
 * TMS Network Functions
 * WiFi and MQTT connection management
 */

#ifndef TMS_NETWORK_H
#define TMS_NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Config.h"

// Global network objects
extern WiFiClient espClient;
extern PubSubClient mqttClient;

// ==================== NETWORK FUNCTIONS ====================

/**
 * Initialize WiFi connection (non-blocking)
 */
void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Non-blocking connection check will be done in mqttTask
}

/**
 * Initialize MQTT client
 */
void setupMQTT();  // Forward declaration, defined in Tasks.h after callback

/**
 * MQTT message callback handler
 */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

/**
 * Setup MQTT client configuration
 */
void setupMQTT() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    Serial.println("MQTT client configured");
}

#endif // TMS_NETWORK_H
