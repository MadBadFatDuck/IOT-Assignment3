package it.unibo.esiot.cus;

import it.unibo.esiot.cus.comm.MQTTService;
import it.unibo.esiot.cus.comm.SerialService;
import it.unibo.esiot.cus.api.HTTPService;
import it.unibo.esiot.cus.service.TankMonitor;
import it.unibo.esiot.cus.model.SystemState;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * CUS - Control Unit Subsystem
 * Main entry point for the Tank Monitoring System backend
 * 
 * This subsystem coordinates:
 * - MQTT communication with TMS (ESP32)
 * - Serial communication with WCS (Arduino)
 * - HTTP server for DBS (Web Dashboard)
 * 
 * Architecture: Multi-threaded with real threading (not virtual threads)
 */
public class CUSMain {

    private static final String CONFIG_FILE = "config.properties";

    private Properties config;
    private SystemState systemState;

    // Service components (each runs in its own thread)
    private MQTTService mqttService;
    private SerialService serialService;
    private HTTPService httpService;
    private TankMonitor tankMonitor;

    public CUSMain() {
        this.config = new Properties();
        this.systemState = new SystemState();
    }

    /**
     * Load configuration from properties file
     */
    private void loadConfiguration() {
        try (InputStream input = getClass().getClassLoader().getResourceAsStream(CONFIG_FILE)) {
            if (input == null) {
                System.err.println("Unable to find " + CONFIG_FILE + ", using defaults");
                setDefaultConfiguration();
                return;
            }
            config.load(input);
            System.out.println("Configuration loaded successfully");
        } catch (IOException e) {
            System.err.println("Error loading configuration: " + e.getMessage());
            setDefaultConfiguration();
        }
    }

    /**
     * Set default configuration values
     */
    private void setDefaultConfiguration() {
        config.setProperty("mqtt.broker", "tcp://localhost:1883");
        config.setProperty("mqtt.topic.level", "tank/level");
        config.setProperty("serial.port", "COM3");
        config.setProperty("serial.baudrate", "9600");
        config.setProperty("http.port", "8080");
        config.setProperty("tank.l1", "20");
        config.setProperty("tank.l2", "40");
        config.setProperty("tank.t1", "10000");
        config.setProperty("tank.t2", "10000");
        config.setProperty("history.size", "100");
    }

    /**
     * Initialize all services
     */
    private void initializeServices() {
        System.out.println("\n=== Initializing CUS Services ===");

        // Initialize MQTT service for TMS communication
        String mqttBroker = config.getProperty("mqtt.broker");
        String mqttTopic = config.getProperty("mqtt.topic.level");
        mqttService = new MQTTService(mqttBroker, mqttTopic, systemState);
        System.out.println("✓ MQTT Service initialized");

        // Initialize Serial service for WCS communication
        String serialPort = config.getProperty("serial.port");
        int baudRate = Integer.parseInt(config.getProperty("serial.baudrate"));
        serialService = new SerialService(serialPort, baudRate, systemState);
        System.out.println("✓ Serial Service initialized");

        // Initialize HTTP service for DBS communication
        int httpPort = Integer.parseInt(config.getProperty("http.port"));
        httpService = new HTTPService(httpPort, systemState, serialService);
        System.out.println("✓ HTTP Service initialized");

        // Initialize Tank Monitor (control logic)
        int l1 = Integer.parseInt(config.getProperty("tank.l1"));
        int l2 = Integer.parseInt(config.getProperty("tank.l2"));
        long t1 = Long.parseLong(config.getProperty("tank.t1"));
        long t2 = Long.parseLong(config.getProperty("tank.t2"));
        tankMonitor = new TankMonitor(systemState, serialService, l1, l2, t1, t2);
        System.out.println("✓ Tank Monitor initialized");

        System.out.println("All services initialized successfully\n");
    }

    /**
     * Start all services in separate threads
     */
    private void startServices() {
        System.out.println("=== Starting CUS Services ===");

        // Start MQTT service thread
        Thread mqttThread = new Thread(mqttService, "MQTT-Thread");
        mqttThread.setDaemon(false);
        mqttThread.start();
        System.out.println("✓ MQTT Service started");

        // Start Serial service thread
        Thread serialThread = new Thread(serialService, "Serial-Thread");
        serialThread.setDaemon(false);
        serialThread.start();
        System.out.println("✓ Serial Service started");

        // Start Tank Monitor thread
        Thread monitorThread = new Thread(tankMonitor, "Monitor-Thread");
        monitorThread.setDaemon(false);
        monitorThread.start();
        System.out.println("✓ Tank Monitor started");

        // Start HTTP service (runs in Javalin's own threads)
        httpService.start();
        System.out.println("✓ HTTP Service started on port " + config.getProperty("http.port"));

        System.out.println("\n=== CUS System Running ===");
        System.out.println("Dashboard available at: http://localhost:" + config.getProperty("http.port"));
        System.out.println("Press Ctrl+C to stop\n");
    }

    /**
     * Shutdown hook for graceful cleanup
     */
    private void registerShutdownHook() {
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("\n=== Shutting down CUS ===");

            if (mqttService != null) {
                mqttService.stop();
                System.out.println("✓ MQTT Service stopped");
            }

            if (serialService != null) {
                serialService.stop();
                System.out.println("✓ Serial Service stopped");
            }

            if (httpService != null) {
                httpService.stop();
                System.out.println("✓ HTTP Service stopped");
            }

            if (tankMonitor != null) {
                tankMonitor.stop();
                System.out.println("✓ Tank Monitor stopped");
            }

            System.out.println("Goodbye!");
        }));
    }

    /**
     * Run the CUS system
     */
    public void run() {
        loadConfiguration();
        initializeServices();
        registerShutdownHook();
        startServices();
    }

    /**
     * Main entry point
     */
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════╗");
        System.out.println("║  CUS - Control Unit Subsystem             ║");
        System.out.println("║  Smart Tank Monitoring System             ║");
        System.out.println("╚════════════════════════════════════════════╝\n");

        CUSMain cus = new CUSMain();
        cus.run();
    }
}
