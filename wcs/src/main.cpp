/**
 * WCS - Water Channel Subsystem
 * Hardware: Arduino UNO
 * Purpose: Control water valve and provide local operator interface
 * Architecture: FSM-based with no blocking delays
 */

#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ==================== CONFIGURATION ====================
// Hardware Pin Configuration
const int SERVO_PIN = 9;
const int POTENTIOMETER_PIN = A0;
const int BUTTON_PIN = 2;

// LCD Configuration (I2C Address 0x27, 16x2 display)
const int LCD_ADDRESS = 0x27;
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// Timing Configuration (non-blocking)
const unsigned long BUTTON_DEBOUNCE_MS = 50;
const unsigned long LCD_UPDATE_INTERVAL_MS = 500;
const unsigned long SERIAL_UPDATE_INTERVAL_MS = 1000;

// Valve Configuration
const int VALVE_MIN_ANGLE = 0;    // 0% = Closed
const int VALVE_MAX_ANGLE = 90;   // 100% = Fully open

// Serial Configuration
const unsigned long SERIAL_BAUD = 9600;

// ==================== GLOBAL OBJECTS ====================
Servo valveServo;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// ==================== FSM STATES ====================
enum SystemMode {
    MODE_UNCONNECTED,   // No connection with CUS
    MODE_AUTOMATIC,     // CUS controls valve
    MODE_MANUAL         // Local control via potentiometer
};

SystemMode currentMode = MODE_UNCONNECTED;
SystemMode previousMode = MODE_UNCONNECTED;

// ==================== GLOBAL STATE VARIABLES ====================
int currentValvePercentage = 0;  // 0-100%
int targetValvePercentage = 0;   // Target from CUS or potentiometer

// Button state
bool lastButtonState = HIGH;
unsigned long lastButtonDebounceTime = 0;

// Timing variables
unsigned long lastLCDUpdate = 0;
unsigned long lastSerialUpdate = 0;
unsigned long lastCUSMessageTime = 0;
const unsigned long CUS_TIMEOUT_MS = 5000;  // 5 seconds without CUS message -> UNCONNECTED

// ==================== FUNCTION PROTOTYPES ====================
// Setup Functions
void setupPins();
void setupServo();
void setupLCD();
void setupSerial();

// FSM Functions
void updateFSM();
void handleModeTransition(SystemMode newMode);

// Input Handling
void handleButtonPress();
int readPotentiometerPercentage();

// Servo Control
void setValvePercentage(int percentage);
int percentageToAngle(int percentage);

// Display Functions
void updateLCD();
void displayModeAndValve();

// Serial Communication
void handleSerialInput();
void sendStatusToSerial();
void processSerialCommand(const String& command);

// ==================== SETUP ====================
void setup() {
    setupSerial();
    Serial.println("\n=== WCS - Water Channel Subsystem ===");
    Serial.println("Initializing...");
    
    setupPins();
    setupServo();
    setupLCD();
    
    // Initial display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WCS Ready");
    lcd.setCursor(0, 1);
    lcd.print("Mode: UNCONN");
    
    delay(1000);
    
    Serial.println("Initialization complete");
    Serial.println("Waiting for CUS connection...");
}

// ==================== MAIN LOOP ====================
void loop() {
    // Non-blocking FSM update
    updateFSM();
    
    // Handle button input
    handleButtonPress();
    
    // Handle serial communication
    handleSerialInput();
    
    // Update LCD periodically
    if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL_MS) {
        updateLCD();
        lastLCDUpdate = millis();
    }
    
    // Send status to CUS periodically
    if (millis() - lastSerialUpdate >= SERIAL_UPDATE_INTERVAL_MS) {
        sendStatusToSerial();
        lastSerialUpdate = millis();
    }
}

// ==================== HARDWARE SETUP ====================
void setupPins() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(POTENTIOMETER_PIN, INPUT);
    Serial.println("Pins configured");
}

void setupServo() {
    valveServo.attach(SERVO_PIN);
    setValvePercentage(0);  // Start with valve closed
    Serial.println("Servo initialized (valve closed)");
}

void setupLCD() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    Serial.println("LCD initialized");
}

void setupSerial() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {
        ; // Wait for serial port to connect (needed for native USB)
    }
}

// ==================== FSM FUNCTIONS ====================
void updateFSM() {
    // Check for CUS timeout
    if (currentMode != MODE_UNCONNECTED) {
        if (millis() - lastCUSMessageTime > CUS_TIMEOUT_MS) {
            handleModeTransition(MODE_UNCONNECTED);
        }
    }
    
    // Execute state-specific logic
    switch (currentMode) {
        case MODE_UNCONNECTED:
            // In unconnected mode, keep valve at last known position
            // Wait for CUS connection
            break;
            
        case MODE_AUTOMATIC:
            // In automatic mode, valve is controlled by CUS via serial
            // Apply target valve percentage received from CUS
            if (currentValvePercentage != targetValvePercentage) {
                setValvePercentage(targetValvePercentage);
            }
            break;
            
        case MODE_MANUAL:
            // In manual mode, read potentiometer and control valve
            targetValvePercentage = readPotentiometerPercentage();
            if (currentValvePercentage != targetValvePercentage) {
                setValvePercentage(targetValvePercentage);
            }
            break;
    }
}

void handleModeTransition(SystemMode newMode) {
    if (currentMode != newMode) {
        Serial.print("Mode transition: ");
        Serial.print(currentMode);
        Serial.print(" -> ");
        Serial.println(newMode);
        
        previousMode = currentMode;
        currentMode = newMode;
        
        // Force LCD update on mode change
        updateLCD();
    }
}

// ==================== INPUT HANDLING ====================
void handleButtonPress() {
    bool buttonState = digitalRead(BUTTON_PIN);
    
    // Check for button state change with debouncing
    if (buttonState != lastButtonState) {
        lastButtonDebounceTime = millis();
    }
    
    if ((millis() - lastButtonDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // Stable button state
        if (buttonState == LOW && lastButtonState == HIGH) {
            // Button pressed (falling edge)
            Serial.println("Button pressed - toggling mode");
            
            // Toggle between AUTOMATIC and MANUAL (ignore if UNCONNECTED)
            if (currentMode == MODE_AUTOMATIC) {
                handleModeTransition(MODE_MANUAL);
            } else if (currentMode == MODE_MANUAL) {
                handleModeTransition(MODE_AUTOMATIC);
            } else {
                Serial.println("Cannot toggle mode - UNCONNECTED");
            }
        }
    }
    
    lastButtonState = buttonState;
}

int readPotentiometerPercentage() {
    int rawValue = analogRead(POTENTIOMETER_PIN);
    // Map 0-1023 to 0-100%
    int percentage = map(rawValue, 0, 1023, 0, 100);
    return constrain(percentage, 0, 100);
}

// ==================== SERVO CONTROL ====================
void setValvePercentage(int percentage) {
    percentage = constrain(percentage, 0, 100);
    int angle = percentageToAngle(percentage);
    
    valveServo.write(angle);
    currentValvePercentage = percentage;
    
    Serial.print("Valve set to: ");
    Serial.print(percentage);
    Serial.println("%");
}

int percentageToAngle(int percentage) {
    return map(percentage, 0, 100, VALVE_MIN_ANGLE, VALVE_MAX_ANGLE);
}

// ==================== DISPLAY FUNCTIONS ====================
void updateLCD() {
    lcd.clear();
    
    // Line 1: Mode
    lcd.setCursor(0, 0);
    lcd.print("Mode: ");
    switch (currentMode) {
        case MODE_UNCONNECTED:
            lcd.print("UNCONN");
            break;
        case MODE_AUTOMATIC:
            lcd.print("AUTO");
            break;
        case MODE_MANUAL:
            lcd.print("MANUAL");
            break;
    }
    
    // Line 2: Valve opening
    lcd.setCursor(0, 1);
    lcd.print("Valve: ");
    lcd.print(currentValvePercentage);
    lcd.print("%");
}

// ==================== SERIAL COMMUNICATION ====================
void handleSerialInput() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            processSerialCommand(command);
            lastCUSMessageTime = millis();  // Reset timeout
            
            // If we were unconnected, transition to automatic
            if (currentMode == MODE_UNCONNECTED) {
                handleModeTransition(MODE_AUTOMATIC);
            }
        }
    }
}

void processSerialCommand(const String& command) {
    // Parse JSON command from CUS
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, command);
    
    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }
    
    const char* cmd = doc["cmd"];
    
    if (strcmp(cmd, "set_valve") == 0) {
        // Command to set valve percentage
        int value = doc["value"];
        Serial.print("Received valve command: ");
        Serial.println(value);
        
        if (currentMode == MODE_AUTOMATIC) {
            targetValvePercentage = value;
        } else {
            Serial.println("Ignored - not in AUTOMATIC mode");
        }
        
    } else if (strcmp(cmd, "set_mode") == 0) {
        // Command to change mode
        const char* mode = doc["value"];
        Serial.print("Received mode command: ");
        Serial.println(mode);
        
        if (strcmp(mode, "AUTOMATIC") == 0) {
            handleModeTransition(MODE_AUTOMATIC);
        } else if (strcmp(mode, "MANUAL") == 0) {
            handleModeTransition(MODE_MANUAL);
        }
    }
}

void sendStatusToSerial() {
    // Send current status to CUS in JSON format
    StaticJsonDocument<200> doc;
    
    switch (currentMode) {
        case MODE_UNCONNECTED:
            doc["mode"] = "UNCONNECTED";
            break;
        case MODE_AUTOMATIC:
            doc["mode"] = "AUTOMATIC";
            break;
        case MODE_MANUAL:
            doc["mode"] = "MANUAL";
            break;
    }
    
    doc["valve"] = currentValvePercentage;
    
    serializeJson(doc, Serial);
    Serial.println();  // End of JSON message
}
