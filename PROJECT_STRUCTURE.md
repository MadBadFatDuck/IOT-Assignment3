# IOT Assignment #3 - Project Structure

This directory contains the complete Smart Tank Monitoring System.

## Directory Structure

```
IOT-Assignment3/
├── tms/                    # Tank Monitoring Subsystem (ESP32)
│   ├── platformio.ini     # PlatformIO configuration
│   └── src/
│       └── main.cpp       # ESP32 firmware with FreeRTOS
│
├── wcs/                    # Water Channel Subsystem (Arduino)
│   ├── platformio.ini     # PlatformIO configuration  
│   └── src/
│       └── main.cpp       # Arduino firmware with FSM
│
├── cus/                    # Control Unit Subsystem (Java Backend)
│   ├── pom.xml            # Maven configuration
│   └── src/main/
│       ├── java/it/unibo/esiot/cus/
│       │   ├── CUSMain.java           # Main entry point
│       │   ├── model/
│       │   │   └── SystemState.java   # Thread-safe state
│       │   ├── comm/
│       │   │   ├── MQTTService.java   # MQTT client
│       │   │   └── SerialService.java # Serial communication
│       │   ├── service/
│       │   │   └── TankMonitor.java   # Control logic
│       │   └── api/
│       │       └── HTTPService.java   # REST API
│       └── resources/
│           └── config.properties      # Configuration
│
├── dbs/                    # Dashboard Subsystem (Web Frontend)
│   ├── index.html         # Main dashboard page
│   ├── style.css          # Styling
│   └── app.js             # JavaScript application logic
│
└── doc/                    # Documentation
    └── (to be created)

```

## Technology Stack

- **TMS**: ESP32 with Arduino Framework (PlatformIO), FreeRTOS, MQTT
- **WCS**: Arduino UNO with Arduino Framework (PlatformIO), FSM, Serial JSON
- **CUS**: Java 21, Maven, Paho MQTT, jSerialComm, Javalin, Gson
- **DBS**: HTML5, CSS3, JavaScript (ES6+), Chart.js, Fetch API

## Prerequisites

### Hardware
- ESP32 development board
- Arduino UNO
- HC-SR04 Sonar sensor
- 2x LEDs (Green, Red)  
- Servo motor
- Potentiometer
- Push button
- 16x2 LCD Display (I2C)
- Breadboards and jumper wires

### Software
- Visual Studio Code with PlatformIO extension
- Java Development Kit (JDK) 21+
- Maven
- MQTT Broker (e.g., Mosquitto)
- Modern web browser

## Setup Instructions

### 1. TMS (ESP32)

1. Open `tms` folder in VS Code with PlatformIO
2. Edit `src/main.cpp`:
   - Set WiFi credentials (WIFI_SSID, WIFI_PASSWORD)
   - Set MQTT broker address
   - Verify pin configurations
3. Connect ESP32 via USB
4. Build and upload: `PlatformIO: Upload`
5. Monitor serial output: `PlatformIO: Serial Monitor`

### 2. WCS (Arduino)

1. Open `wcs` folder in VS Code with PlatformIO
2. Verify pin configurations in `src/main.cpp`
3. Connect Arduino UNO via USB
4. Build and upload: `PlatformIO: Upload`
5. Monitor serial output: `PlatformIO: Serial Monitor`

### 3. CUS (Backend)

1. Navigate to `cus` folder
2. Edit `src/main/resources/config.properties`:
   - Set MQTT broker address
   - Set correct serial port for Arduino
   - Adjust parameters (L1, L2, T1, T2)
3. Compile: `mvn clean compile`
4. Run: `mvn exec:java -Dexec.mainClass="it.unibo.esiot.cus.CUSMain"`

### 4. DBS (Dashboard)

1. Edit `dbs/app.js`:
   - Set API_BASE_URL if CUS is not on localhost:8080
2. Open `dbs/index.html` in a web browser
3. Or serve with a simple HTTP server:
   ```bash
   cd dbs
   python -m http.server 3000
   ```
   Then navigate to `http://localhost:3000`

## Running the Complete System

1. **Start MQTT Broker** (if not running):
   ```bash
   mosquitto -v
   ```

2. **Flash and power TMS (ESP32)**:
   - Will automatically connect to WiFi and MQTT

3. **Flash and connect WCS (Arduino)**:
   - Connect to PC via USB (same port as configured in CUS)

4. **Start CUS Backend**:
   ```bash
   cd cus
   mvn exec:java -Dexec.mainClass="it.unibo.esiot.cus.CUSMain"
   ```

5. **Open Dashboard**:
   - Open `dbs/index.html` in browser

## Testing

- **Automatic Mode**: Move object above sonar to simulate water level changes
- **Manual Mode**: Press button on Arduino to switch to manual, use potentiometer to control valve
- **Network Failure**: Disconnect ESP32 WiFi to test UNCONNECTED state
- **Dashboard**: All controls and visualizations should work in real-time

## Troubleshooting

- **Serial Port**: Check Windows Device Manager for correct COM port
- **MQTT Connection**: Ensure broker is running and accessible
- **Compilation Errors**: Run `mvn clean install` to download dependencies
- **Dashboard Not Updating**: Check browser console for errors, verify CUS is running

## Configuration Parameters

Default values in `config.properties`:
- **L1**: 20 cm (first threshold)
- **L2**: 40 cm (critical threshold)
- **T1**: 10000 ms (10 seconds)
- **T2**: 10000 ms (10 seconds)
- **F**: 1 Hz (1 second sampling)

Adjust these values based on your tank size and requirements.


