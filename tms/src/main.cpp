/**
 * TMS - Tank Monitoring Subsystem
 * Hardware: ESP32
 * Purpose: Monitor rainwater level using sonar and publish data via MQTT
 * Architecture: FreeRTOS Task-based with FSM for network state management
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==================== CONFIGURATION ====================
// WiFi Configuration
const char* WIFI_SSID = "TP-LINK_53CACA";
const char* WIFI_PASSWORD = "65363331";

// MQTT Configuration
const char* MQTT_BROKER = "localhost";  // Change to your MQTT broker IP
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC_LEVEL = "tank/level";
const char* MQTT_CLIENT_ID = "TMS_ESP32";

// Hardware Pin Configuration
const int SONAR_TRIG_PIN = 13;
const int SONAR_ECHO_PIN = 12;
const int LED_GREEN_PIN = 14;  // Network OK
const int LED_RED_PIN = 27;    // Network Error

// Timing Configuration
const int SAMPLING_FREQUENCY_MS = 1000;  // 1 Hz (F parameter)
const int RECONNECT_DELAY_MS = 5000;

// ==================== GLOBAL OBJECTS ====================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ==================== FSM STATES ====================
enum SystemState {
    STATE_INITIALIZING,
    STATE_CONNECTING_WIFI,
    STATE_CONNECTING_MQTT,
    STATE_CONNECTED,
    STATE_NETWORK_ERROR
};

volatile SystemState currentState = STATE_INITIALIZING;

// ==================== TASK HANDLES ====================
TaskHandle_t sonarTaskHandle = NULL;
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t ledTaskHandle = NULL;

// ==================== FUNCTION PROTOTYPES ====================
// Setup Functions
void setupPins();
void setupWiFi();
void setupMQTT();

// FSM Functions
void updateSystemState();
void handleStateTransition(SystemState newState);

// Sensor Functions
float readSonarDistance();

// Network Functions
void connectWiFi();
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// FreeRTOS Task Functions
void sonarTask(void* parameter);
void mqttTask(void* parameter);
void ledTask(void* parameter);

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== TMS - Tank Monitoring Subsystem ===");
    Serial.println("Initializing...");
    
    // Initialize hardware pins
    setupPins();
    
    // Create FreeRTOS Tasks
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
    
    // Initial state transition
    handleStateTransition(STATE_CONNECTING_WIFI);
}

// ==================== MAIN LOOP ====================
void loop() {
    // Empty - all work done in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// ==================== HARDWARE SETUP ====================
void setupPins() {
    pinMode(SONAR_TRIG_PIN, OUTPUT);
    pinMode(SONAR_ECHO_PIN, INPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    
    Serial.println("Pins configured");
}

// ==================== NETWORK SETUP ====================
void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Non-blocking connection check will be done in mqttTask
}

void setupMQTT() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    Serial.println("MQTT client configured");
}

// ==================== FSM FUNCTIONS ====================
void handleStateTransition(SystemState newState) {
    if (currentState != newState) {
        Serial.print("State transition: ");
        Serial.print(currentState);
        Serial.print(" -> ");
        Serial.println(newState);
        
        currentState = newState;
    }
}

// ==================== SENSOR FUNCTIONS ====================
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

// ==================== NETWORK FUNCTIONS ====================
void connectWiFi() {
    // TODO: Implement non-blocking WiFi connection logic
    // Check WiFi.status() and handle connection
}

void connectMQTT() {
    // TODO: Implement MQTT connection logic
    // Use mqttClient.connect() and handle result
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // TODO: Handle incoming MQTT messages (if needed)
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

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
                
                // TODO: Store reading for MQTT task to publish
                // You can use a queue or shared variable with mutex
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
                    
                    // TODO: Publish water level data
                    // char msg[50];
                    // sprintf(msg, "%.2f", waterLevel);
                    // mqttClient.publish(MQTT_TOPIC_LEVEL, msg);
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
 */
void ledTask(void* parameter) {
    for (;;) {
        switch (currentState) {
            case STATE_CONNECTED:
                digitalWrite(LED_GREEN_PIN, HIGH);
                digitalWrite(LED_RED_PIN, LOW);
                break;
                
            case STATE_NETWORK_ERROR:
                digitalWrite(LED_GREEN_PIN, LOW);
                digitalWrite(LED_RED_PIN, HIGH);
                break;
                
            default:
                // Blink both LEDs during initialization/connection
                digitalWrite(LED_GREEN_PIN, !digitalRead(LED_GREEN_PIN));
                digitalWrite(LED_RED_PIN, !digitalRead(LED_RED_PIN));
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
