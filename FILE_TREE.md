# Smart Tank Monitoring System - Complete File Tree

```
IOT-Assignment3/
│
├── 📁 tms/ ────────────────────── Tank Monitoring Subsystem (ESP32)
│   ├── platformio.ini           PlatformIO config with ESP32 + libraries
│   └── src/
│       └── main.cpp              FreeRTOS tasks, MQTT, Sonar, LED control
│
├── 📁 wcs/ ────────────────────── Water Channel Subsystem (Arduino UNO)
│   ├── platformio.ini           PlatformIO config with Arduino + libraries
│   └── src/
│       └── main.cpp              FSM, Servo, LCD, Button, Serial JSON
│
├── 📁 cus/ ────────────────────── Control Unit Subsystem (Java Backend)
│   ├── pom.xml                   Maven config (Paho MQTT, jSerialComm, Javalin, Gson)
│   └── src/main/
│       ├── java/it/unibo/esiot/cus/
│       │   ├── CUSMain.java                    ✓ Main + Thread management
│       │   │
│       │   ├── model/
│       │   │   └── SystemState.java            ✓ Thread-safe state with R/W locks
│       │   │
│       │   ├── comm/
│       │   │   ├── MQTTService.java            ✓ MQTT subscriber (TMS data)
│       │   │   └── SerialService.java          ✓ Serial I/O (WCS commands)
│       │   │
│       │   ├── service/
│       │   │   └── TankMonitor.java            ✓ L1/L2/T1/T2 control logic
│       │   │
│       │   └── api/
│       │       └── HTTPService.java            ✓ REST API (Javalin)
│       │
│       └── resources/
│           └── config.properties               Configuration file
│
├── 📁 dbs/ ────────────────────── Dashboard Subsystem (Web Frontend)
│   ├── index.html                Modern HTML5 dashboard structure
│   ├── style.css                 Dark theme with glassmorphism & animations
│   └── app.js                    Real-time updates, Chart.js, Fetch API
│
├── 📁 doc/
│   └── (documentation to be created)
│
├── PROJECT_STRUCTURE.md          Complete setup instructions
└── README.md                     Project overview

```

## Key Features Implemented

### ✅ TMS (ESP32) - `tms/src/main.cpp`
- **FreeRTOS Tasks**: Sonar task, MQTT task, LED task
- **FSM States**: INITIALIZING → CONNECTING_WIFI → CONNECTING_MQTT → CONNECTED ↔ NETWORK_ERROR
- **MQTT Publishing**: Water level data to `tank/level` topic
- **LED Indicators**: Green (connected), Red (error), Blinking (connecting)
- **Libraries**: WiFi, PubSubClient (MQTT)

### ✅ WCS (Arduino UNO) - `wcs/src/main.cpp`
- **FSM States**: UNCONNECTED ↔ AUTOMATIC ↔ MANUAL
- **Servo Control**: 0° = closed, 90° = fully open (0-100%)
- **LCD Display**: Shows mode and valve percentage (16x2 I2C)
- **Button Control**: Toggle AUTOMATIC ↔ MANUAL (debounced)
- **Potentiometer**: Manual valve control in MANUAL mode
- **Serial Protocol**: JSON bidirectional communication
- **Libraries**: Servo, LiquidCrystal_I2C, ArduinoJson

### ✅ CUS (Java Backend) - 6 Classes
1. **CUSMain.java**: Entry point, creates 3 real threads + Javalin threads
2. **SystemState.java**: Thread-safe state with ReadWriteLock
3. **MQTTService.java**: Subscribes to TMS data, auto-reconnect
4. **SerialService.java**: Bidirectional JSON with Arduino
5. **TankMonitor.java**: Implements L1/L2/T1/T2 policy  
6. **HTTPService.java**: REST API (GET status/history, POST mode/valve)

**Dependencies (pom.xml)**:
- `org.eclipse.paho.client.mqttv3` - MQTT client
- `com.fazecast.jSerialComm` - Serial communication
- `io.javalin` - Lightweight web server
- `com.google.code.gson` - JSON processing

### ✅ DBS (Web Dashboard) - 3 Files
- **index.html**: Responsive grid layout with Chart.js integration
- **style.css**: Premium dark theme with gradients, animations, glassmorphism
- **app.js**: Real-time polling (1s), mode control, manual valve slider

**Features**:
- Real-time water level graph (Chart.js line chart)
- Circular gauge for valve opening (SVG animation)
- System state badge (AUTOMATIC/MANUAL/UNCONNECTED/NOT AVAILABLE)
- Mode toggle buttons
- Manual valve control slider (enabled only in MANUAL mode)

## Communication Protocols

```
┌─────────┐  MQTT (WiFi)   ┌─────────┐  Serial (JSON)  ┌─────────┐
│   TMS   │ ─────────────→ │   CUS   │ ←────────────→ │   WCS   │
│ (ESP32) │  tank/level    │ (Java)  │  {cmd,value}    │(Arduino)│
└─────────┘                └─────────┘                 └─────────┘
                                ↕
                           HTTP REST API
                                ↕
                           ┌─────────┐
                           │   DBS   │
                           │  (Web)  │
                           └─────────┘
```

## Configuration Parameters

**Defined in `cus/src/main/resources/config.properties`:**

| Parameter | Default | Description |
|-----------|---------|-------------|
| `mqtt.broker` | tcp://localhost:1883 | MQTT broker address |
| `mqtt.topic.level` | tank/level | TMS water level topic |
| `serial.port` | COM3 | Arduino serial port |
| `serial.baudrate` | 9600 | Serial communication speed |
| `http.port` | 8080 | REST API port |
| `tank.l1` | 20 cm | First level threshold |
| `tank.l2` | 40 cm | Critical level threshold |
| `tank.t1` | 10000 ms | Time before 50% opening |
| `tank.t2` | 10000 ms | TMS timeout |
| `history.size` | 100 | Historical data points |

## Compilation Status

All subsystems are ready to compile:
- **TMS**: Open in PlatformIO, upload to ESP32
- **WCS**: Open in PlatformIO, upload to Arduino UNO
- **CUS**: `mvn clean compile` (downloads dependencies automatically)
- **DBS**: Static files, open `index.html` in browser

## Next Steps

1. **Hardware Assembly**: Connect components to ESP32 and Arduino
2. **Configuration**: Update WiFi credentials, MQTT broker, serial port
3. **Testing**: Flash firmware, start backend, open dashboard
4. **Documentation**: Create FSM diagrams, breadboard schemas, report, video

---

**Note**: All code includes extensive comments and TODOs for easy understanding and extension. The architecture follows best practices with separation of concerns, thread safety, and non-blocking I/O.
