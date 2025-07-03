#include <Arduino.h>
#include "config/config.h"

// Interface headers for component abstraction
#include "devices/api/LedStatus.h"
#include "devices/api/TemperatureManager.h"
#include "kernel/connection/api/WifiManager.h"
#include "kernel/connection/api/MqttManager.h"
#include "kernel/api/IFsmManager.h"

// Implementation headers
#include "devices/api/LedStatusImpl.h"
#include "devices/api/TemperatureManagerImpl.h"
#include "kernel/connection/api/WifiManagerImpl.h"
#include "kernel/connection/api/MqttManagerImpl.h"
#include "kernel/api/FsmManagerImpl.h"

// Pointers to interfaces for component decoupling
LedStatus* ledStatus = nullptr;
TemperatureManager* temperatureManager = nullptr;
WifiManager* wifiManager = nullptr;
MqttManager* mqttManager = nullptr;
IFsmManager* systemFsm = nullptr;

/**
 * @brief Setup function, runs once at system startup.
 * 
 * Initializes serial communication, creates component instances,
 * sets up individual modules, creates and initializes the FSM.
 */
void setup() {
    Serial.begin(115200);
    while (!Serial) { ; } // Wait for serial port to be ready
    Serial.println("\n\n[Smart Temperature Monitor - ESP32] System Starting...");

    // Create instances of concrete implementations
    ledStatus = new LedStatusImpl();
    temperatureManager = new TemperatureManagerImpl();
    wifiManager = new WifiManagerImpl(WIFI_SSID, WIFI_PASSWORD);
    mqttManager = new MqttManagerImpl(MQTT_SERVER_HOST, MQTT_SERVER_PORT, MQTT_CLIENT_ID_PREFIX, wifiManager);

    // Create FSM instance, passing references to required modules
    systemFsm = new FsmManagerImpl(*ledStatus, *temperatureManager, *wifiManager, *mqttManager);

    // Setup individual modules
    ledStatus->setup();
    ledStatus->indicateSystemBoot();
    temperatureManager->setup();
    wifiManager->setup();
    mqttManager->setup();

    // Setup FSM
    systemFsm->setup();

    Serial.println("System initialization completed.");
}

/**
 * @brief Main loop function, runs repeatedly.
 * 
 * Manages MQTT communication loop and executes the FSM's run cycle.
 * The FSM handles all state management and coordination between components.
 */
void loop() {
    // MQTT loop must be called regularly to maintain connection
    // and process incoming/outgoing messages
    if (wifiManager && wifiManager->isConnected()) {
        if (mqttManager) {
            mqttManager->loop();
        }
    }

    // Execute one FSM cycle
    if (systemFsm) {
        systemFsm->run();
    }
}