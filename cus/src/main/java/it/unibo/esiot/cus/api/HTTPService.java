package it.unibo.esiot.cus.api;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import io.javalin.Javalin;
import io.javalin.http.Context;
import it.unibo.esiot.cus.comm.SerialService;
import it.unibo.esiot.cus.model.SystemState;
import it.unibo.esiot.cus.model.SystemState.WaterLevelReading;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * HTTPService - REST API for Dashboard Communication
 * 
 * Provides HTTP endpoints for the web dashboard (DBS)
 * Uses Javalin framework
 */
public class HTTPService {

    private final int port;
    private final SystemState systemState;
    private final SerialService serialService;
    private final Gson gson;

    private Javalin app;

    public HTTPService(int port, SystemState systemState, SerialService serialService) {
        this.port = port;
        this.systemState = systemState;
        this.serialService = serialService;
        this.gson = new Gson();
    }

    /**
     * Start the HTTP server
     */
    public void start() {
        app = Javalin.create(config -> {
            // Enable CORS for web dashboard
            config.bundledPlugins.enableCors(cors -> {
                cors.addRule(rule -> {
                    rule.anyHost();
                });
            });
        });

        // Define routes
        app.get("/api/status", this::getStatus);
        app.get("/api/history", this::getHistory);
        app.post("/api/mode", this::setMode);
        app.post("/api/valve", this::setValve);

        // Static files for dashboard (if serving from here)
        // app.staticFiles.add("/public");

        app.start(port);
        System.out.println("[HTTP] Server started on port " + port);
    }

    /**
     * GET /api/status
     * Returns current system status
     */
    private void getStatus(Context ctx) {
        Map<String, Object> status = new HashMap<>();

        SystemState.Mode mode = systemState.getCurrentMode();
        status.put("mode", mode.toString());
        status.put("waterLevel", systemState.getCurrentWaterLevel());
        status.put("valveOpening", systemState.getCurrentValveOpening());
        status.put("tmsConnected", systemState.isTMSConnected(10000));
        status.put("timestamp", System.currentTimeMillis());

        ctx.json(status);
    }

    /**
     * GET /api/history
     * Returns historical water level data
     */
    private void getHistory(Context ctx) {
        List<WaterLevelReading> history = systemState.getLevelHistory();

        Map<String, Object> response = new HashMap<>();
        response.put("readings", history);
        response.put("count", history.size());

        ctx.json(response);
    }

    /**
     * POST /api/mode
     * Switch system mode (AUTOMATIC / MANUAL)
     * 
     * Request body: {"mode": "AUTOMATIC"} or {"mode": "MANUAL"}
     */
    private void setMode(Context ctx) {
        try {
            JsonObject request = gson.fromJson(ctx.body(), JsonObject.class);
            String modeStr = request.get("mode").getAsString().toUpperCase();

            SystemState.Mode newMode = SystemState.Mode.valueOf(modeStr);

            // Cannot manually set to UNCONNECTED
            if (newMode == SystemState.Mode.UNCONNECTED) {
                ctx.status(400).json(Map.of("error", "Cannot manually set UNCONNECTED mode"));
                return;
            }

            System.out.println("[HTTP] Mode change request: " + newMode);

            // Update system state
            systemState.setCurrentMode(newMode);

            // Send mode change to WCS
            serialService.sendModeCommand(modeStr);

            ctx.json(Map.of("success", true, "mode", modeStr));

        } catch (Exception e) {
            ctx.status(400).json(Map.of("error", "Invalid mode: " + e.getMessage()));
        }
    }

    /**
     * POST /api/valve
     * Set valve opening in MANUAL mode
     * 
     * Request body: {"opening": 75}
     */
    private void setValve(Context ctx) {
        try {
            JsonObject request = gson.fromJson(ctx.body(), JsonObject.class);
            int opening = request.get("opening").getAsInt();

            // Validate range
            if (opening < 0 || opening > 100) {
                ctx.status(400).json(Map.of("error", "Opening must be 0-100"));
                return;
            }

            // Only allow in MANUAL mode
            if (systemState.getCurrentMode() != SystemState.Mode.MANUAL) {
                ctx.status(400).json(Map.of("error", "Can only set valve in MANUAL mode"));
                return;
            }

            System.out.println("[HTTP] Manual valve control: " + opening + "%");

            // Send command to WCS
            serialService.sendValveCommand(opening);
            systemState.setCurrentValveOpening(opening);

            ctx.json(Map.of("success", true, "opening", opening));

        } catch (Exception e) {
            ctx.status(400).json(Map.of("error", "Invalid request: " + e.getMessage()));
        }
    }

    /**
     * Stop the HTTP server
     */
    public void stop() {
        if (app != null) {
            app.stop();
            System.out.println("[HTTP] Server stopped");
        }
    }
}
