/**
 * @file main.cpp
 * @brief Main application for the Smart Temperature Monitoring System.
 * 
 * This file contains the setup() and loop() functions for the ESP32.
 * It initializes all hardware components (LEDs, temperature sensor),
 * network managers (WiFi, MQTT), and the finite state machine (FSM),
 * then repeatedly executes the system logic in the main loop.
 */

#include <Arduino.h>
#include "config/config.h"

// Interface headers for component abstraction
#include "devices/api/LedStatus.h"
#include "devices/api/TemperatureManager.h"
#include "kernel/connection/api/WifiManager.h"
#include "kernel/connection/api/MqttManager.h"
#include "kernel/api/IFsmManager.h"

// Implementation headers (needed for instantiation)
#include "devices/api/LedStatusImpl.h"
#include "devices/api/TemperatureManagerImpl.h"
#include "kernel/connection/api/WifiManagerImpl.h"
#include "kernel/connection/api/MqttManagerImpl.h"
#include "kernel/api/FsmManagerImpl.h"

// === Pointers to interfaces for component decoupling ===
LedStatus* ledStatus = nullptr;
TemperatureManager* temperatureManager = nullptr;
WifiManager* wifiManager = nullptr;
MqttManager* mqttManager = nullptr;
IFsmManager* systemFsm = nullptr;

// === Configuration Constants ===
// These could be moved to config.h if shared across files
const char* WIFI_NETWORK_SSID = WIFI_SSID;
const char* WIFI_NETWORK_PASSWORD = WIFI_PASSWORD;
const char* MQTT_BROKER_HOST = MQTT_SERVER_HOST;
const int MQTT_BROKER_PORT = MQTT_SERVER_PORT;
const char* MQTT_CLIENT_PREFIX = MQTT_CLIENT_ID_PREFIX;

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
    wifiManager = new WifiManagerImpl(WIFI_NETWORK_SSID, WIFI_NETWORK_PASSWORD);
    mqttManager = new MqttManagerImpl(MQTT_BROKER_HOST, MQTT_BROKER_PORT, MQTT_CLIENT_PREFIX, wifiManager);

    // Create FSM instance, passing references to required modules
    systemFsm = new FsmManagerImpl(*ledStatus, *temperatureManager, *wifiManager, *mqttManager);

    // Setup individual modules
    ledStatus->setup();
    ledStatus->indicateSystemBoot();
    temperatureManager->setup();
    wifiManager->setup();
    mqttManager->setup();

    // Setup FSM (may perform additional state initialization)
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