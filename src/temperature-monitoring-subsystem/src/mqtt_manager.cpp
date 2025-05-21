#include "mqtt_manager.h"
#include "config.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h> 

// Definizioni delle costanti da config.h
const char* MQTT_SERVER_HOST = "test.mosquitto.org";
const int MQTT_SERVER_PORT = 1883;
const char* MQTT_CLIENT_ID_PREFIX = "esp32s3-temp-mon-";
const char* MQTT_TOPIC_TEMPERATURE = "assignment3/temperature";
const char* MQTT_TOPIC_STATUS = "assignment3/status";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
String actualMqttClientId;


// Funzione di callback MQTT (non usata attivamente per ora, ma pronta)
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT: Messaggio ricevuto su topic [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Esempio: se ricevi un comando per cambiare la frequenza
  // if (String(topic) == MQTT_TOPIC_CONFIG_F) {
  //   long new_interval = message.toInt();
  //   if (new_interval > 0) {
  //     // Aggiorna TEMP_SAMPLE_INTERVAL_MS (dovrebbe essere gestito in modo sicuro)
  //   }
  // }
}

void setupMQTT() {
  // Genera un Client ID univoco aggiungendo un numero random o parte del MAC address
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  actualMqttClientId = String(MQTT_CLIENT_ID_PREFIX) + String(chipId, HEX);

  mqttClient.setServer(MQTT_SERVER_HOST, MQTT_SERVER_PORT);
  mqttClient.setCallback(mqttCallback); // Imposta la callback
  Serial.print("MQTT: Setup completato. Client ID: ");
  Serial.println(actualMqttClientId);
}

bool connectMQTT() {
  if (mqttClient.connected()) {
    return true;
  }
  Serial.print("MQTT: Tentativo di connessione al broker ");
  Serial.print(MQTT_SERVER_HOST);
  Serial.print(" come ");
  Serial.println(actualMqttClientId);
  
  if (mqttClient.connect(actualMqttClientId.c_str())) {
    Serial.println("MQTT: Connesso!");
    // Iscriviti ai topic necessari qui (es. per la frequenza F)
    // mqttClient.subscribe(MQTT_TOPIC_CONFIG_F);
    publishStatus("ESP32 Online");
    return true;
  } else {
    Serial.print("MQTT: Connessione fallita, rc=");
    Serial.print(mqttClient.state());
    // mqttClient.state() codes:
    // -4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
    // -3 : MQTT_CONNECTION_LOST - the network connection was broken
    // -2 : MQTT_CONNECT_FAILED - the network connection failed
    // -1 : MQTT_DISCONNECTED - the client is disconnected cleanly
    // 0 : MQTT_CONNECTED - the client is connected
    // 1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
    // 2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
    // 3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
    // 4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
    // 5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
    Serial.println(" Riproverò tra poco...");
    return false;
  }
}

void disconnectMQTT() {
    mqttClient.disconnect();
    Serial.println("MQTT: Disconnesso.");
}

bool isMQTTConnected() {
  return mqttClient.connected();
}

void mqttLoop() {
  if (isWiFiConnected()) { // Assicurati che il WiFi sia attivo prima di chiamare loop MQTT
    mqttClient.loop(); // Essenziale per mantenere attiva la connessione e processare messaggi
  }
}

bool publishTemperature(float temperature) {
  if (!isMQTTConnected()) {
    Serial.println("MQTT: Impossibile pubblicare, non connesso.");
    return false;
  }
  char tempString[8];
  dtostrf(temperature, 1, 2, tempString); // Converte float in stringa (valore, min_width, decimal_places, buffer)
  
  Serial.print("MQTT: Invio temperatura '");
  Serial.print(tempString);
  Serial.print("' al topic '");
  Serial.print(MQTT_TOPIC_TEMPERATURE);
  Serial.println("'");

  if (mqttClient.publish(MQTT_TOPIC_TEMPERATURE, tempString, true)) { // true per retained message
    Serial.println("MQTT: Temperatura inviata con successo.");
    return true;
  } else {
    Serial.println("MQTT: Errore invio temperatura.");
    return false;
  }
}

bool publishStatus(const char* statusMessage) {
  if (!isMQTTConnected()) {
    Serial.println("MQTT: Impossibile pubblicare stato, non connesso.");
    return false;
  }
  Serial.print("MQTT: Invio stato '");
  Serial.print(statusMessage);
  Serial.print("' al topic '");
  Serial.print(MQTT_TOPIC_STATUS);
  Serial.println("'");

  if (mqttClient.publish(MQTT_TOPIC_STATUS, statusMessage)) {
    Serial.println("MQTT: Stato inviato con successo.");
    return true;
  } else {
    Serial.println("MQTT: Errore invio stato.");
    return false;
  }
}