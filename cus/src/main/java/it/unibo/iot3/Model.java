package it.unibo.iot3;

/**
 * Rappresenta lo stato del sistema (Single Source of Truth).
 */
public class Model {

    // States
    public enum SystemMode { AUTOMATIC, MANUAL, UNCONNECTED }

    private double waterLevel = 0;      // Livello inviato dal TMS via MQTT
    private int valveOpening = 0;      // Apertura 0-100% gestita dal CUS o WCS
    private SystemMode mode = SystemMode.AUTOMATIC;
    private long lastUpdateTimestamp;  // Utile per gestire il timeout T2

    public Model() {
        this.lastUpdateTimestamp = System.currentTimeMillis();
    }

    // syncronyzed perchè è L/S contemporanea

    public synchronized void setWaterLevel(double level) {
        this.waterLevel = level;
        this.lastUpdateTimestamp = System.currentTimeMillis();
    }

    public synchronized double getWaterLevel() {
        return waterLevel;
    }

    public synchronized void setValveOpening(int opening) {
        // Vtra 0 e 100
        this.valveOpening = Math.max(0, Math.min(100, opening));
    }

    public synchronized int getValveOpening() {
        return valveOpening;
    }

    public synchronized void setMode(SystemMode mode) {
        this.mode = mode;
    }

    public synchronized SystemMode getMode() {
        return mode;
    }

    public synchronized long getLastUpdateTimestamp() {
        return lastUpdateTimestamp;
    }
}