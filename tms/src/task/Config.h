/**
 * TMS Configuration
 * All system configuration constants and pin definitions
 */

#ifndef TMS_CONFIG_H
#define TMS_CONFIG_H

// ==================== WIFI CONFIGURATION ====================
const char* WIFI_SSID = "TP-LINK_53CACA";
const char* WIFI_PASSWORD = "65363331";

// ==================== MQTT CONFIGURATION ====================
const char* MQTT_BROKER = "192.168.0.101";  // Change to your MQTT broker IP
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC_LEVEL = "tank/level";
const char* MQTT_CLIENT_ID = "TMS_ESP32";

// ==================== HARDWARE PIN CONFIGURATION ====================
const int SONAR_TRIG_PIN = 13;
const int SONAR_ECHO_PIN = 12;
const int LED_GREEN_PIN = 2;  // Network OK
const int LED_RED_PIN = 4;    // Network Error

// ==================== TIMING CONFIGURATION ====================
const int SAMPLING_FREQUENCY_MS = 1000;  // 1 Hz (F parameter)
const int RECONNECT_DELAY_MS = 5000;

#endif // TMS_CONFIG_H
