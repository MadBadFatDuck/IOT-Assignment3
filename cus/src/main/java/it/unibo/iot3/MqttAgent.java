package it.unibo.iot3;

import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

public class MqttAgent {
    private final String broker = "tcp://broker.mqtt-dashboard.com:1883";
    private final String clientId = "CUS_Control_Unit_" + System.currentTimeMillis();
    private final String topic = "esiot/assignments/03/tank/level";
    private MqttClient client;
    private Model model;

    public MqttAgent(Model model) {
        this.model = model;
    }

    public void start() {
        try {
            client = new MqttClient(broker, clientId, new MemoryPersistence());
            MqttConnectOptions options = new MqttConnectOptions();
            options.setCleanSession(true);
            options.setAutomaticReconnect(true);

            client.connect(options);
            System.out.println("MQTT: Connesso al broker " + broker);

            client.subscribe(topic, (t, msg) -> {
                String payload = new String(msg.getPayload());
                try {
                    double level = Double.parseDouble(payload);
                    model.setWaterLevel(level);
                    System.out.println("MQTT: Nuovo livello ricevuto -> " + level);
                } catch (NumberFormatException e) {
                    System.err.println("MQTT: Messaggio non valido ricevuto: " + payload);
                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }
}