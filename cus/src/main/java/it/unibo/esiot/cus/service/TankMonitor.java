package it.unibo.esiot.cus.service;

import it.unibo.esiot.cus.comm.SerialService;
import it.unibo.esiot.cus.model.SystemState;
import it.unibo.esiot.cus.model.SystemState.Mode;

/**
 * TankMonitor - Core Control Logic
 * 
 * Implements the tank monitoring policy based on water levels L1, L2
 * and time thresholds T1, T2
 * Runs in its own thread
 */
public class TankMonitor implements Runnable {

    private final SystemState systemState;
    private final SerialService serialService;

    // Configuration parameters
    private final int L1; // First level threshold (cm)
    private final int L2; // Critical level threshold (cm)
    private final long T1; // Time before opening at 50% (ms)
    private final long T2; // Timeout for unconnected state (ms)

    // Internal state for timing
    private long levelAboveL1Since = 0;
    private boolean wasAboveL1 = false;

    private volatile boolean running = false;

    private static final long UPDATE_INTERVAL_MS = 500; // Check every 500ms

    public TankMonitor(SystemState systemState, SerialService serialService,
            int l1, int l2, long t1, long t2) {
        this.systemState = systemState;
        this.serialService = serialService;
        this.L1 = l1;
        this.L2 = l2;
        this.T1 = t1;
        this.T2 = t2;

        System.out.println("[TankMonitor] Configuration:");
        System.out.println("  L1 = " + L1 + " cm");
        System.out.println("  L2 = " + L2 + " cm");
        System.out.println("  T1 = " + T1 + " ms");
        System.out.println("  T2 = " + T2 + " ms");
    }

    @Override
    public void run() {
        running = true;
        System.out.println("[TankMonitor] Started monitoring");

        try {
            while (running) {
                updateControlLogic();
                Thread.sleep(UPDATE_INTERVAL_MS);
            }
        } catch (InterruptedException e) {
            System.out.println("[TankMonitor] Thread interrupted");
            Thread.currentThread().interrupt();
        }

        System.out.println("[TankMonitor] Stopped");
    }

    /**
     * Main control logic - implements the tank monitoring policy
     */
    private void updateControlLogic() {
        // Check if TMS is connected
        if (!systemState.isTMSConnected(T2)) {
            // No data from TMS for T2 time -> UNCONNECTED state
            if (systemState.getCurrentMode() != Mode.UNCONNECTED) {
                System.out.println("[TankMonitor] TMS timeout - entering UNCONNECTED state");
                systemState.setCurrentMode(Mode.UNCONNECTED);
            }
            return;
        }

        // Only apply automatic control if in AUTOMATIC mode
        if (systemState.getCurrentMode() != Mode.AUTOMATIC) {
            return;
        }

        // Get current water level
        float currentLevel = systemState.getCurrentWaterLevel();

        // Apply control policy
        int targetValveOpening = calculateValveOpening(currentLevel);

        // Send command to WCS if valve opening needs to change
        if (targetValveOpening != systemState.getCurrentValveOpening()) {
            System.out.println(
                    "[TankMonitor] Setting valve to " + targetValveOpening + "% (level: " + currentLevel + " cm)");
            serialService.sendValveCommand(targetValveOpening);
            systemState.setCurrentValveOpening(targetValveOpening);
        }
    }

    /**
     * Calculate required valve opening based on water level
     * 
     * Policy:
     * - If level >= L2: Open 100% immediately
     * - If L1 < level < L2 for >= T1 time: Open 50%
     * - If level <= L1: Close (0%)
     */
    private int calculateValveOpening(float level) {
        // Critical level - open immediately at 100%
        if (level >= L2) {
            resetTimers();
            return 100;
        }

        // Below L1 - close valve
        if (level <= L1) {
            resetTimers();
            return 0;
        }

        // Between L1 and L2 - check timing
        if (level > L1 && level < L2) {
            if (!wasAboveL1) {
                // Just crossed L1 threshold
                levelAboveL1Since = System.currentTimeMillis();
                wasAboveL1 = true;
                System.out.println("[TankMonitor] Level above L1, starting timer");
            }

            long timeAboveL1 = System.currentTimeMillis() - levelAboveL1Since;

            if (timeAboveL1 >= T1) {
                // Been above L1 for T1 time - open at 50%
                return 50;
            } else {
                // Still within T1 period - keep valve as is
                return systemState.getCurrentValveOpening();
            }
        }

        return 0; // Default: closed
    }

    /**
     * Reset timing state
     */
    private void resetTimers() {
        if (wasAboveL1) {
            wasAboveL1 = false;
            levelAboveL1Since = 0;
        }
    }

    /**
     * Stop the tank monitor
     */
    public void stop() {
        running = false;
    }
}
