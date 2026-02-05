# 💧 Smart Tank Monitoring System - IoT Assignment #03

## Panoramica del Progetto

Il **Smart Tank Monitoring System** è un sistema IoT modulare progettato per monitorare in tempo reale il livello di pioggia in un serbatoio e gestire l'apertura automatica o manuale di un canale di drenaggio collegato a una rete idrica.

Il sistema è composto da quattro sottosistemi interconnessi tramite diversi protocolli standard (MQTT, HTTP, Serial) per garantire flessibilità e scalabilità.


### Prerequisiti

* **Hardware**: ESP32/ESP8266, Arduino UNO, Sonar, Servo, LED, Potenziometro, Pulsante, Display LCD.
* **Software**:
    * Arduino IDE o PlatformIO (per TMS e WCS).
    * Ambiente di sviluppo per il Back-end CUS (e.g., Python/Node.js/Java).
    * Broker MQTT attivo (e.g., Mosquitto).

### Setup e Avvio

1.  **WCS (Arduino)**: Caricare il firmware in `src/wcs` sull'Arduino UNO. Assicurarsi che i pin del Servo, LCD e Potenziometro siano configurati correttamente.
2.  **TMS (ESP)**: Caricare il firmware in `src/tms` sull'ESP. Inserire le credenziali Wi-Fi e l'indirizzo del Broker MQTT.
3.  **CUS (Back-end)**: Eseguire il server in `src/cus`. Configurare le porte MQTT, HTTP e la porta **Seriale** utilizzata dall'Arduino.
4.  **DBS (Front-end)**: Avviare l'applicazione web/dashboard in `src/dbs`. Configurare l'endpoint HTTP del server CUS.

---