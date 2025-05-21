#include <Arduino.h>
#include "config.h"
#include "fsm_manager.h"
#include "led_status.h"
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"

// Definizione della variabile di stato globale (dichiarata extern in fsm_manager.h)
SystemState currentState = STATE_INITIALIZING;

// Variabili globali per la FSM e temporizzazioni
unsigned long lastTempSampleTime = 0;
unsigned long lastMqttAttemptTime = 0;
unsigned long lastWiFiAttemptTime = 0;
float currentTemperature = 0.0;
// Inizializza con il default da config.h, ma verrà sovrascritto se arriva un comando MQTT
unsigned long currentSamplingIntervalMs = TEMP_SAMPLE_INTERVAL_DEFAULT_MS;


void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("\n\n[Smart Temp Monitor - ESP32] Avvio Sistema...");

  setupLed();             // Chiama la nuova setupLed()
  indicateSystemBoot();   // Nuova funzione per indicare il boot con LED esterni

  setupSensor();
  setupWiFi();
  setupMQTT();

  Serial.println("Setup iniziale completato.");
  currentState = STATE_INITIALIZING;
  lastWiFiAttemptTime = millis();
}


void loop() {
  unsigned long currentTime = millis();

  if (isWiFiConnected()) {
    mqttLoop();
  }

  // Controlla se è stato ricevuto un nuovo intervallo di campionamento
  unsigned long newInterval = getNewSamplingIntervalMs(); // Da mqtt_manager
  if (newInterval > 0) {
      currentSamplingIntervalMs = newInterval;
      Serial.print("Main: Intervallo di campionamento aggiornato dal Control Unit a: ");
      Serial.print(currentSamplingIntervalMs);
      Serial.println(" ms");

  switch (currentState) {
    case STATE_INITIALIZING:
      Serial.println("FSM: STATE_INITIALIZING -> Tentativo connessione WiFi");
      indicateWifiConnecting(); 
      if (connectWiFi()) {
        currentState = STATE_WIFI_CONNECTED;
        Serial.println("FSM: WiFi Connesso -> STATE_WIFI_CONNECTED");
      } else {
        Serial.println("FSM: WiFi Connessione iniziale fallita -> STATE_NETWORK_ERROR");
        indicateNetworkError(); 
        currentState = STATE_NETWORK_ERROR;
        lastWiFiAttemptTime = currentTime;
      }
      break;

    case STATE_WIFI_CONNECTED:
      Serial.println("FSM: STATE_WIFI_CONNECTED -> Tentativo connessione MQTT");
      indicateMqttConnecting(); 
      currentState = STATE_MQTT_CONNECTING;
      lastMqttAttemptTime = 0;
      break;

    case STATE_MQTT_CONNECTING:
      if (isMQTTConnected()) {
        Serial.println("FSM: MQTT Connesso -> STATE_OPERATIONAL");
        indicateOperational(); 
        currentState = STATE_OPERATIONAL;
      } else if (currentTime - lastMqttAttemptTime >= MQTT_RECONNECT_INTERVAL_MS) {
        Serial.println("FSM: Riprovo connessione MQTT...");
        indicateMqttConnecting(); 
        if (!connectMQTT()) {
             Serial.println("FSM: Tentativo MQTT fallito, attendo prossimo intervallo.");
        }
        lastMqttAttemptTime = currentTime;
      }
      if (!isWiFiConnected()) {
        Serial.println("FSM: WiFi perso durante tentativo MQTT -> STATE_NETWORK_ERROR");
        indicateNetworkError(); 
        currentState = STATE_NETWORK_ERROR;
        lastWiFiAttemptTime = currentTime;
      }
      break;

    case STATE_OPERATIONAL:
      indicateOperational(); 
      if (!isWiFiConnected() || !isMQTTConnected()) {
        Serial.println("FSM: Connessione persa (WiFi o MQTT) -> STATE_NETWORK_ERROR");
        indicateNetworkError(); 
        currentState = STATE_NETWORK_ERROR;
        lastWiFiAttemptTime = currentTime;
        break;
      }
      // Usa currentSamplingIntervalMs che potrebbe essere stato aggiornato
      if (currentTime - lastTempSampleTime >= currentSamplingIntervalMs) { 
        currentState = STATE_SAMPLING_TEMPERATURE;
        Serial.println("FSM: -> STATE_SAMPLING_TEMPERATURE");
      }
      break;

    case STATE_SAMPLING_TEMPERATURE:
      Serial.println("FSM: Campionamento temperatura...");
      currentTemperature = readTemperature();
      Serial.print("Temperatura: ");
      Serial.print(currentTemperature);
      Serial.println(" C");
      lastTempSampleTime = currentTime;
      currentState = STATE_SENDING_DATA;
      Serial.println("FSM: -> STATE_SENDING_DATA");
      break;

    case STATE_SENDING_DATA:
      Serial.println("FSM: Invio dati temperatura...");
      if (publishTemperature(currentTemperature)) {
        Serial.println("FSM: Dati inviati.");
      } else {
        Serial.println("FSM: Fallito invio dati. MQTT potrebbe essere disconnesso.");
      }
      currentState = STATE_OPERATIONAL;
      Serial.println("FSM: -> STATE_OPERATIONAL (dopo invio)");
      break;

    case STATE_NETWORK_ERROR:
      indicateNetworkError(); 
      Serial.println("FSM: Errore di rete. In attesa per ritentare connessione generale...");
      if(isMQTTConnected()) disconnectMQTT();
      currentState = STATE_WAIT_RECONNECT;
      Serial.println("FSM: -> STATE_WAIT_RECONNECT");
      lastWiFiAttemptTime = currentTime;
      break;

    case STATE_WAIT_RECONNECT:
      indicateNetworkError(); 
      if (currentTime - lastWiFiAttemptTime >= WIFI_RECONNECT_INTERVAL_MS) {
        Serial.println("FSM: Fine attesa, ritento connessione WiFi (tornando a INITIALIZING)...");
        currentState = STATE_INITIALIZING;
      }
      break;

    default:
      Serial.println("FSM: Stato sconosciuto! Ritorno a INITIALIZING.");
      indicateNetworkError(); 
      currentState = STATE_INITIALIZING;
      break;
  }
  delay(100);
}}