// src/main.cpp
#include <Arduino.h>
#include "config/config.h" // Per le costanti globali e intervalli di default

// Includi le INTERFACCE dei moduli e della FSM
#include "devices/api/LedStatus.h"
#include "devices/api/TemperatureManager.h"
#include "kernel/connection/api/WifiManager.h"
#include "kernel/connection/api/MqttManager.h"
#include "kernel/api/IFsmManager.h"    // << NUOVA INTERFACCIA FSM

// Includi le IMPLEMENTAZIONI (per poter creare le istanze)
#include "devices/api/LedStatusImpl.h"
#include "devices/api/TemperatureManagerImpl.h"
#include "kernel/connection/api/WifiManagerImpl.h"
#include "kernel/connection/api/MqttManagerImpl.h"
#include "kernel/api/FsmManagerImpl.h" // << NUOVA IMPLEMENTAZIONE FSM

// === Puntatori alle interfacce per i moduli del sistema ===
LedStatus* ledStatus = nullptr;
TemperatureManager* temperatureManager = nullptr;
WifiManager* wifiManager = nullptr;
MqttManager* mqttManager = nullptr;
IFsmManager* systemFsm = nullptr; // << PUNTATORE ALL'INTERFACCIA FSM

// === Variabili di Configurazione (definite qui e passate ai costruttori) ===
const char* MY_WIFI_SSID = "TIM-18202156";
const char* MY_WIFI_PASSWORD = "MbeCOCznKXXVA7j1wzT4tapt";
const char* MY_MQTT_SERVER_HOST = "192.168.1.100";
const int   MY_MQTT_SERVER_PORT = 1883;
const char* MY_MQTT_CLIENT_ID_PREFIX = "esp32s3-oop-fsm-";


/**
 * @brief Setup function, runs once at system startup.
 * Initializes serial communication, module instances (LED, Sensor, WiFi, MQTT),
 * and the FSM instance. Then, setups up the FSM.
 */
void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    Serial.println("\n\n[Smart Temp Monitor - ESP32 OOP FSM] Avvio Sistema...");

    // Crea istanze delle implementazioni concrete.
    ledStatus = new LedStatusImpl();
    temperatureManager = new TemperatureManagerImpl();
    wifiManager = new WifiManagerImpl(MY_WIFI_SSID, MY_WIFI_PASSWORD);
    mqttManager = new MqttManagerImpl(MY_MQTT_SERVER_HOST, MY_MQTT_SERVER_PORT, MY_MQTT_CLIENT_ID_PREFIX, wifiManager);

    // Crea l'istanza della FSM, passando i riferimenti ai moduli necessari
    systemFsm = new FsmManagerImpl(*ledStatus, *temperatureManager, *wifiManager, *mqttManager);

    // Esegui il setup per ogni modulo DIRETTO (non tramite FSM)
    ledStatus->setup();
    ledStatus->indicateSystemBoot();
    temperatureManager->setup();
    wifiManager->setup();
    mqttManager->setup();

    // Esegui il setup della FSM (che potrebbe fare ulteriori inizializzazioni di stato)
    systemFsm->setup();

    Serial.println("Setup OOP FSM iniziale completato.");
}

/**
 * @brief Main loop function, runs repeatedly.
 * Manages MQTT communication loop and executes the FSM's run cycle.
 */
void loop() {
    // Il loop MQTT deve essere chiamato regolarmente per mantenere la connessione
    // e processare i messaggi in entrata/uscita.
    if (wifiManager && wifiManager->isConnected()) {
        if (mqttManager) {
            mqttManager->loop();
        }
    }

    // Esegui un ciclo della FSM.
    if (systemFsm) {
        systemFsm->run();
    }

    delay(100); // Mantieni un breve delay per stabilità.
}