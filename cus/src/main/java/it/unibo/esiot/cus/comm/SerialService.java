package it.unibo.esiot.cus.comm;

import com.fazecast.jSerialComm.SerialPort;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import it.unibo.esiot.cus.model.SystemState;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;

/**
 * SerialService - Serial Communication with WCS (Arduino)
 * 
 * Sends valve control commands and receives status updates
 * Runs in its own thread
 */
public class SerialService implements Runnable {

    private final String portName;
    private final int baudRate;
    private final SystemState systemState;

    private SerialPort serialPort;
    private BufferedReader reader;
    private PrintWriter writer;
    private final Gson gson;

    private volatile boolean running = false;

    public SerialService(String portName, int baudRate, SystemState systemState) {
        this.portName = portName;
        this.baudRate = baudRate;
        this.systemState = systemState;
        this.gson = new Gson();
    }

    @Override
    public void run() {
        running = true;

        if (!openSerialPort()) {
            System.err.println("[Serial] Failed to open serial port: " + portName);
            System.err.println("[Serial] Available ports:");
            SerialPort[] ports = SerialPort.getCommPorts();
            for (SerialPort port : ports) {
                System.err.println("  - " + port.getSystemPortName());
            }
            return;
        }

        try {
            long lastHeartbeat = System.currentTimeMillis();
            final long HEARTBEAT_INTERVAL_MS = 2000; // Send heartbeat every 2 seconds

            // Main loop - read incoming messages from WCS and send heartbeat
            // Main loop - read incoming messages from WCS and send heartbeat
            while (running) {
                try {
                    String line = reader.readLine(); // Blocking read
                    if (line != null && !line.trim().isEmpty()) {
                        handleIncomingMessage(line.trim());
                    }
                } catch (Exception e) {
                    // Timeout is expected if no data, just continue
                }

                // Send periodic heartbeat to keep WCS connection alive
                long now = System.currentTimeMillis();
                if (now - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
                    sendHeartbeat();
                    lastHeartbeat = now;
                }
            }
        } catch (Exception e) {
            System.err.println("[Serial] Error: " + e.getMessage());
            e.printStackTrace();
        } finally {
            closeSerialPort();
        }
    }

    /**
     * Open and configure serial port
     */
    private boolean openSerialPort() {
        try {
            serialPort = SerialPort.getCommPort(portName);
            serialPort.setBaudRate(baudRate);
            serialPort.setNumDataBits(8);
            serialPort.setNumStopBits(1);
            serialPort.setParity(SerialPort.NO_PARITY);
            serialPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 100, 0);

            if (serialPort.openPort()) {
                System.out.println("[Serial] Port opened: " + portName + " @ " + baudRate + " baud");

                // Initialize reader and writer
                reader = new BufferedReader(new InputStreamReader(serialPort.getInputStream()));
                writer = new PrintWriter(serialPort.getOutputStream(), true);

                return true;
            } else {
                System.err.println("[Serial] Failed to open port: " + portName);
                return false;
            }
        } catch (Exception e) {
            System.err.println("[Serial] Error opening port: " + e.getMessage());
            return false;
        }
    }

    /**
     * Handle incoming message from WCS
     */
    private void handleIncomingMessage(String message) {
        try {
            System.out.println("[Serial] Received: " + message);

            // Parse JSON message
            JsonObject json = gson.fromJson(message, JsonObject.class);

            if (json.has("mode")) {
                String mode = json.get("mode").getAsString();
                System.out.println("[Serial] WCS Mode: " + mode);
                if (mode.equals("AUTOMATIC")) {
                    systemState.setCurrentMode(SystemState.Mode.AUTOMATIC);
                } else if (mode.equals("MANUAL")) {
                    systemState.setCurrentMode(SystemState.Mode.MANUAL);
                } else if (mode.equals("UNCONNECTED")) {
                    systemState.setCurrentMode(SystemState.Mode.UNCONNECTED);
                }
            }

            if (json.has("valve")) {
                int valve = json.get("valve").getAsInt();
                System.out.println("[Serial] WCS Valve: " + valve + "%");
                // Update system state with actual valve position
                systemState.setCurrentValveOpening(valve);
            }

        } catch (Exception e) {
            System.err.println("[Serial] Error parsing message: " + e.getMessage());
        }
    }

    /**
     * Send valve control command to WCS
     * This method is thread-safe and can be called from other threads
     */
    public synchronized void sendValveCommand(int percentage) {
        if (writer == null) {
            System.err.println("[Serial] Cannot send command - port not open");
            return;
        }

        try {
            JsonObject command = new JsonObject();
            command.addProperty("cmd", "set_valve");
            command.addProperty("value", percentage);

            String json = gson.toJson(command);
            writer.println(json);
            writer.flush();

            System.out.println("[Serial] Sent valve command: " + percentage + "%");
        } catch (Exception e) {
            System.err.println("[Serial] Error sending valve command: " + e.getMessage());
        }
    }

    /**
     * Send mode change command to WCS
     * This method is thread-safe and can be called from other threads
     */
    public synchronized void sendModeCommand(String mode) {
        if (writer == null) {
            System.err.println("[Serial] Cannot send command - port not open");
            return;
        }

        try {
            JsonObject command = new JsonObject();
            command.addProperty("cmd", "set_mode");
            command.addProperty("value", mode);

            String json = gson.toJson(command);
            writer.println(json);
            writer.flush();

            System.out.println("[Serial] Sent mode command: " + mode);
        } catch (Exception e) {
            System.err.println("[Serial] Error sending mode command: " + e.getMessage());
        }
    }

    /**
     * Send heartbeat ping to WCS to keep connection alive
     * This method is thread-safe and can be called from other threads
     */
    public synchronized void sendHeartbeat() {
        if (writer == null) {
            return; // Silently fail if port not open
        }

        try {
            JsonObject command = new JsonObject();
            command.addProperty("cmd", "ping");

            String json = gson.toJson(command);
            writer.println(json);
            writer.flush();

            // Don't log heartbeat to avoid spam - uncomment for debugging
            // System.out.println("[Serial] Sent heartbeat");
        } catch (Exception e) {
            System.err.println("[Serial] Error sending heartbeat: " + e.getMessage());
        }
    }

    /**
     * Stop the serial service
     */
    public void stop() {
        running = false;
    }

    /**
     * Close serial port
     */
    private void closeSerialPort() {
        if (serialPort != null && serialPort.isOpen()) {
            serialPort.closePort();
            System.out.println("[Serial] Port closed");
        }
    }
}
