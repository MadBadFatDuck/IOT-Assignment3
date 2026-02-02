package it.unibo.esiot.cus.model;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * SystemState - Thread-safe central state management
 * 
 * Maintains the current state of the entire Tank Monitoring System
 * Shared between all services (MQTT, Serial, HTTP, Monitor)
 */
public class SystemState {

    /**
     * System operating modes
     */
    public enum Mode {
        AUTOMATIC, // CUS controls valve automatically
        MANUAL, // Operator controls valve manually
        UNCONNECTED // No connection with TMS
    }

    // Thread-safety lock
    private final ReadWriteLock lock = new ReentrantReadWriteLock();

    // Current system state
    private Mode currentMode;
    private float currentWaterLevel; // in cm
    private int currentValveOpening; // 0-100%
    private long lastTMSMessageTime; // timestamp of last TMS message

    // Historical data
    private final List<WaterLevelReading> levelHistory;
    private final int maxHistorySize;

    /**
     * Data class for water level readings
     */
    public static class WaterLevelReading {
        public final float level;
        public final long timestamp;

        public WaterLevelReading(float level, long timestamp) {
            this.level = level;
            this.timestamp = timestamp;
        }
    }

    /**
     * Constructor
     */
    public SystemState() {
        this(100); // Default history size
    }

    public SystemState(int maxHistorySize) {
        this.maxHistorySize = maxHistorySize;
        this.levelHistory = new ArrayList<>();
        this.currentMode = Mode.UNCONNECTED;
        this.currentWaterLevel = 0.0f;
        this.currentValveOpening = 0;
        this.lastTMSMessageTime = 0;
    }

    // ==================== GETTERS (Thread-safe) ====================

    public Mode getCurrentMode() {
        lock.readLock().lock();
        try {
            return currentMode;
        } finally {
            lock.readLock().unlock();
        }
    }

    public float getCurrentWaterLevel() {
        lock.readLock().lock();
        try {
            return currentWaterLevel;
        } finally {
            lock.readLock().unlock();
        }
    }

    public int getCurrentValveOpening() {
        lock.readLock().lock();
        try {
            return currentValveOpening;
        } finally {
            lock.readLock().unlock();
        }
    }

    public long getLastTMSMessageTime() {
        lock.readLock().lock();
        try {
            return lastTMSMessageTime;
        } finally {
            lock.readLock().unlock();
        }
    }

    public List<WaterLevelReading> getLevelHistory() {
        lock.readLock().lock();
        try {
            return new ArrayList<>(levelHistory); // Return a copy
        } finally {
            lock.readLock().unlock();
        }
    }

    // ==================== SETTERS (Thread-safe) ====================

    public void setCurrentMode(Mode mode) {
        lock.writeLock().lock();
        try {
            this.currentMode = mode;
            System.out.println("[SystemState] Mode changed to: " + mode);
        } finally {
            lock.writeLock().unlock();
        }
    }

    public void setCurrentWaterLevel(float level) {
        lock.writeLock().lock();
        try {
            this.currentWaterLevel = level;
            this.lastTMSMessageTime = System.currentTimeMillis();

            // Add to history
            levelHistory.add(new WaterLevelReading(level, lastTMSMessageTime));

            // Remove oldest if exceeds max size
            if (levelHistory.size() > maxHistorySize) {
                levelHistory.remove(0);
            }

            System.out.println("[SystemState] Water level updated: " + level + " cm");
        } finally {
            lock.writeLock().unlock();
        }
    }

    public void setCurrentValveOpening(int opening) {
        lock.writeLock().lock();
        try {
            this.currentValveOpening = Math.max(0, Math.min(100, opening));
            System.out.println("[SystemState] Valve opening updated: " + this.currentValveOpening + "%");
        } finally {
            lock.writeLock().unlock();
        }
    }

    // ==================== UTILITY METHODS ====================

    /**
     * Check if TMS connection is active based on timeout
     */
    public boolean isTMSConnected(long timeoutMs) {
        lock.readLock().lock();
        try {
            if (lastTMSMessageTime == 0) {
                return false; // Never received a message
            }
            long timeSinceLastMessage = System.currentTimeMillis() - lastTMSMessageTime;
            return timeSinceLastMessage < timeoutMs;
        } finally {
            lock.readLock().unlock();
        }
    }

    /**
     * Get system state as a string for debugging
     */
    @Override
    public String toString() {
        lock.readLock().lock();
        try {
            return String.format("SystemState{mode=%s, level=%.2f cm, valve=%d%%, tmsConnected=%b}",
                    currentMode, currentWaterLevel, currentValveOpening,
                    isTMSConnected(10000));
        } finally {
            lock.readLock().unlock();
        }
    }
}
