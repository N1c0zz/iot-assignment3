#include "mqtt_manager.h"
#include "config.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h> 

// Definizioni delle costanti da config.h
const char* MQTT_SERVER_HOST = "localhost";
const int MQTT_SERVER_PORT = 1883;
const char* MQTT_CLIENT_ID_PREFIX = "esp32s3-temp-mon-";
const char* MQTT_TOPIC_TEMPERATURE = "assignment3/temperature";
const char* MQTT_TOPIC_STATUS = "assignment3/status";
const char* MQTT_TOPIC_CONFIG_F = "assignment3/frequency";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
String actualMqttClientId;

// Variabile per memorizzare il nuovo intervallo ricevuto, e un flag per indicare se è nuovo
static unsigned long newSamplingIntervalReceived = 0;
static bool newIntervalAvailable = false;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT: Messaggio ricevuto su topic [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (String(topic) == MQTT_TOPIC_CONFIG_F) {
    Serial.print("MQTT: Ricevuto comando frequenza: ");
    Serial.println(message);
    long new_interval_val = message.toInt();
    if (new_interval_val >= 1000 && new_interval_val <= 600000) { // Es: tra 1 secondo e 10 minuti
      newSamplingIntervalReceived = new_interval_val;
      newIntervalAvailable = true; // Imposta il flag
      Serial.print("MQTT: Nuovo intervallo di campionamento ricevuto e valido: ");
      Serial.println(newSamplingIntervalReceived);
    } else {
      Serial.println("MQTT: Valore intervallo non valido ricevuto.");
    }
  }
}

void setupMQTT() {
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  actualMqttClientId = String(MQTT_CLIENT_ID_PREFIX) + String(chipId, HEX);

  mqttClient.setServer(MQTT_SERVER_HOST, MQTT_SERVER_PORT);
  mqttClient.setCallback(mqttCallback);
  Serial.print("MQTT: Setup completato. Client ID: ");
  Serial.println(actualMqttClientId);
  Serial.print("MQTT: Server: ");
  Serial.print(MQTT_SERVER_HOST);
  Serial.print(":");
  Serial.println(MQTT_SERVER_PORT);
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
    publishStatus("ESP32 Online");

    // Sottoscrizione al topic per la configurazione della frequenza
    Serial.print("MQTT: Sottoscrizione al topic di configurazione frequenza: ");
    Serial.println(MQTT_TOPIC_CONFIG_F);
    if (mqttClient.subscribe(MQTT_TOPIC_CONFIG_F)) {
        Serial.println("MQTT: Sottoscrizione avvenuta con successo.");
    } else {
        Serial.println("MQTT: Errore durante la sottoscrizione al topic di frequenza.");
    }
    return true;
  } else {
    Serial.print("MQTT: Connessione fallita, rc=");
    Serial.print(mqttClient.state());
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
    Serial.println("MQTT: Impossibile pubblicare temperatura, non connesso.");
    return false;
  }
  char tempString[8];
  dtostrf(temperature, 1, 2, tempString);
  
  Serial.print("MQTT: Invio temperatura '");
  Serial.print(tempString);
  Serial.print("' al topic '");
  Serial.print(MQTT_TOPIC_TEMPERATURE);
  Serial.println("'");

  if (mqttClient.publish(MQTT_TOPIC_TEMPERATURE, tempString, true)) { // true per retained
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

unsigned long getNewSamplingIntervalMs() {
  if (newIntervalAvailable) {
    newIntervalAvailable = false; // Resetta il flag dopo aver letto il valore
    return newSamplingIntervalReceived;
  }
  return 0; // Nessun nuovo intervallo
}