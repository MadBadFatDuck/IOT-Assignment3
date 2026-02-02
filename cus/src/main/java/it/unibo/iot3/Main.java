import io.javalin.Javalin;
import it.unibo.iot3.Model;
import it.unibo.iot3.MqttAgent;

public static void main(String[] args) {
    // 1. Inizializza il Modello (Stato)
    Model model = new Model();

    // 2. Avvia l'agente MQTT per ricevere dati dal TMS
    MqttAgent mqtt = new MqttAgent(model);
    mqtt.start();

    // 3. Avvia il server HTTP per la Dashboard
    var app = Javalin.create().start(8080);

    // Endpoint per leggere lo stato attuale dalla Dashboard
    app.get("/api/status", ctx -> {
        ctx.json(model); // Restituisce un JSON
    });

    System.out.println("CUS Online: HTTP su 8080, MQTT in ascolto...");
}