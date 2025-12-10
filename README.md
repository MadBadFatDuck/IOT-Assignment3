# 💧 Smart Tank Monitoring System - IoT Assignment #03

## Panoramica del Progetto

Il **Smart Tank Monitoring System** è un sistema IoT modulare progettato per monitorare in tempo reale il livello di pioggia in un serbatoio e gestire l'apertura automatica o manuale di un canale di drenaggio collegato a una rete idrica.

Il sistema è composto da quattro sottosistemi interconnessi tramite diversi protocolli standard (MQTT, HTTP, Serial) per garantire flessibilità e scalabilità.



## Architettura del Sistema

Il sistema gestisce due modalità operative principali (`AUTOMATIC` e `MANUAL`) e monitora la connettività per gestire gli stati di errore (`UNCONNECTED` / `NOT AVAILABLE`).

### 1. Tank Monitoring Subsystem (TMS) - `src/tms`
* **Piattaforma**: ESP32 / ESP8266 (SoC)
* **Ruolo**: Acquisizione dati e monitoraggio dello stato di rete.
* **Componenti**: Sonar (livello acqua), LED Verde/Rosso (stato connettività).
* **Logica**: Implementata tramite **Finite State Machine (FSM)**.
* **Comunicazione**: Invia i dati di livello al CUS tramite **MQTT**.

### 2. Control Unit Subsystem (CUS) - `src/cus`
* **Piattaforma**: PC (Server/Back-end)
* **Ruolo**: Core decisionale. Implementa la politica di controllo (livelli $L_1$, $L_2$, tempi $T_1$, $T_2$) e coordina tutti gli altri sottosistemi.
* **Logica**: Gestisce lo stato globale del sistema e la logica di controllo `AUTOMATIC`.
* **Comunicazione**:
    * **MQTT** con TMS.
    * **Serial Line** con WCS (per invio comando apertura).
    * **HTTP** con DBS (per telemetria e comandi remoti).

### 3. Water Channels Subsystem (WCS) - `src/wcs`
* **Piattaforma**: Arduino UNO (Microcontroller)
* **Ruolo**: Controllo fisico della valvola e interfaccia locale per l'operatore.
* **Componenti**: Servo Motore (valvola), Potenziometro (controllo manuale), Pulsante (cambio modo), Display LCD (stato locale).
* **Logica**: Implementata tramite **FSM** per gestire il cambio di modalità `AUTOMATIC`/`MANUAL` tramite pulsante e l'attuazione del servo.
* **Comunicazione**: Scambia dati e comandi con il CUS tramite **Serial Line**.

### 4. Dashboard Subsystem (DBS) - `src/dbs`
* **Piattaforma**: PC (Front-end/Web App)
* **Ruolo**: Interfaccia operatore remota per visualizzazione e controllo.
* **Funzionalità**: Grafico storico livello acqua, stato sistema, pulsante cambio modo, widget di controllo manuale.
* **Comunicazione**: Interagisce con il CUS tramite **HTTP**.

## Dettagli sulla Logica di Controllo

### Politica AUTOMATIC (gestita dal CUS)

La logica si basa su due soglie di livello pioggia ($L_1 < L_2$) e due soglie temporali ($T_1$, $T_2$):

| Condizione | Apertura Valvola | Note |
| :--- | :--- | :--- |
| $\text{Level} \geq L_2$ | **100%** | Apertura immediata. |
| $L_1 < \text{Level} < L_2$ per $\geq T_1$ tempo | **50%** | Apertura dopo un periodo di allerta. |
| $\text{Level} \leq L_1$ | **0%** | Valvola chiusa (stato normale). |
| Assenza dati TMS per $\geq T_2$ tempo | Passa allo stato **UNCONNECTED** | Sicurezza e gestione errori di rete. |

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