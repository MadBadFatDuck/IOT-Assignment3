package it.unibo.esiot.cus.comm;

import it.unibo.esiot.cus.model.SystemState;
import org.eclipse.paho.client.mqttv3.*;

/**
 * MQTTService - MQTT Client for TMS Communication
 * 
 * Subscribes to water level data from the TMS (ESP32)
 * Runs in its own thread
 */
public class MQTTService implements Runnable {

    private final String brokerUrl;
    private final String topic;
    private final SystemState systemState;

    private MqttClient mqttClient;
    private volatile boolean running = false;

    public MQTTService(String brokerUrl, String topic, SystemState systemState) {
        this.brokerUrl = brokerUrl;
        this.topic = topic;
        this.systemState = systemState;
    }

    @Override
    public void run() {
        running = true;

        try {
            // Create MQTT client
            String clientId = "CUS_" + System.currentTimeMillis();
            mqttClient = new MqttClient(brokerUrl, clientId);

            // Set connection options
            MqttConnectOptions options = new MqttConnectOptions();
            options.setCleanSession(true);
            options.setAutomaticReconnect(true);
            options.setConnectionTimeout(10);
            options.setKeepAliveInterval(20);

            // Set callback for incoming messages
            mqttClient.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    System.err.println("[MQTT] Connection lost: " + cause.getMessage());
                    // Automatic reconnect is enabled, so it will try to reconnect
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    handleIncomingMessage(topic, message);
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    // Not used for subscriber
                }
            });

            // Connect to broker
            System.out.println("[MQTT] Connecting to broker: " + brokerUrl);
            mqttClient.connect(options);
            System.out.println("[MQTT] Connected successfully");

            // Subscribe to topic
            mqttClient.subscribe(topic, 1); // QoS 1
            System.out.println("[MQTT] Subscribed to topic: " + topic);

            // Keep thread alive
            while (running) {
                Thread.sleep(1000);
            }

        } catch (MqttException e) {
            System.err.println("[MQTT] Error: " + e.getMessage());
            e.printStackTrace();
        } catch (InterruptedException e) {
            System.out.println("[MQTT] Thread interrupted");
            Thread.currentThread().interrupt();
        } finally {
            disconnect();
        }
    }

    /**
     * Handle incoming MQTT message from TMS
     */
    private void handleIncomingMessage(String topic, MqttMessage message) {
        try {
            String payload = new String(message.getPayload());
            System.out.println("[MQTT] Received: " + payload);

            // Parse water level (expecting just a number as string)
            float waterLevel = Float.parseFloat(payload.trim());

            // Update system state
            systemState.setCurrentWaterLevel(waterLevel);

            // If we were unconnected and now receiving data, switch to automatic
            if (systemState.getCurrentMode() == SystemState.Mode.UNCONNECTED) {
                systemState.setCurrentMode(SystemState.Mode.AUTOMATIC);
            }

        } catch (NumberFormatException e) {
            System.err.println("[MQTT] Invalid message format: " + new String(message.getPayload()));
        }
    }

    /**
     * Stop the MQTT service
     */
    public void stop() {
        running = false;
        disconnect();
    }

    /**
     * Disconnect from MQTT broker
     */
    private void disconnect() {
        if (mqttClient != null && mqttClient.isConnected()) {
            try {
                mqttClient.disconnect();
                mqttClient.close();
                System.out.println("[MQTT] Disconnected");
            } catch (MqttException e) {
                System.err.println("[MQTT] Error disconnecting: " + e.getMessage());
            }
        }
    }
}
