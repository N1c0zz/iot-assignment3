#include "mqtt_manager.h"
#include "config.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>

// Definizioni delle costanti da config.h
const char* MQTT_SERVER_HOST = "192.168.1.100";
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

  // Copia il payload in un buffer per ArduinoJson (perché payload potrebbe non essere null-terminated)
  char jsonBuffer[256]; // Assicurati che sia abbastanza grande per il tuo JSON
  if (length < sizeof(jsonBuffer) -1) { // -1 per il terminatore null
      memcpy(jsonBuffer, payload, length);
      jsonBuffer[length] = '\0'; // Aggiungi il terminatore null
      Serial.println(jsonBuffer); // Stampa il JSON ricevuto
  } else {
      Serial.println("Errore: Payload JSON troppo grande per il buffer!");
      return;
  }


  if (String(topic) == MQTT_TOPIC_CONFIG_F) {
    Serial.print("MQTT: Ricevuto comando frequenza (JSON): ");
    Serial.println(jsonBuffer);

    // Parsa il JSON
    StaticJsonDocument<128> doc; // Scegli una dimensione appropriata per il tuo JSON
                                 // Per {"frequency": XXXXXX} 128 è abbondante
    DeserializationError error = deserializeJson(doc, jsonBuffer);

    if (error) {
      Serial.print("MQTT: deserializeJson() fallito: ");
      Serial.println(error.f_str());
      return;
    }

    // Estrai il valore della frequenza
    // Il valore nel JSON è in secondi (60), ma noi lo vogliamo in millisecondi
    if (doc.containsKey("frequency")) {
        long frequency_seconds = doc["frequency"]; // Estrae il valore 60
        unsigned long new_interval_val_ms = frequency_seconds * 1000; // Converti in millisecondi

        Serial.print("MQTT: Frequenza letta (secondi): ");
        Serial.print(frequency_seconds);
        Serial.print(", Convertita in ms: ");
        Serial.println(new_interval_val_ms);

        // Validazione (ora sui millisecondi)
        if (new_interval_val_ms >= 1000 && new_interval_val_ms <= 600000) { // Es: tra 1 secondo e 10 minuti
          newSamplingIntervalReceived = new_interval_val_ms;
          newIntervalAvailable = true;
          Serial.print("MQTT: Nuovo intervallo di campionamento ricevuto e valido: ");
          Serial.println(newSamplingIntervalReceived);
        } else {
          Serial.println("MQTT: Valore intervallo (convertito in ms) non valido.");
        }
    } else {
        Serial.println("MQTT: Chiave 'frequency' non trovata nel JSON.");
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

  StaticJsonDocument<100> jsonDoc; // Scegli una dimensione appropriata. 100 dovrebbe bastare per {"temperature": xx.yy}

  jsonDoc["temperature"] = temperature; // Arrotonda automaticamente se necessario, o usa round() prima se vuoi un controllo preciso

  char jsonBuffer[100]; // Buffer per la stringa JSON
  size_t n = serializeJson(jsonDoc, jsonBuffer); // serializeJson restituisce il numero di byte scritti

  Serial.print("MQTT: Invio JSON '");
  Serial.print(jsonBuffer); // Stampa es. {"temperature":23.50}
  Serial.print("' al topic '");
  Serial.print(MQTT_TOPIC_TEMPERATURE);
  Serial.println("'");

  if (mqttClient.publish(MQTT_TOPIC_TEMPERATURE, reinterpret_cast<const uint8_t*>(jsonBuffer), n, true)) { // Invia jsonBuffer con la sua lunghezza n, true per retained
    Serial.println("MQTT: Temperatura JSON inviata con successo.");
    return true;
  } else {
    Serial.println("MQTT: Errore invio temperatura JSON.");
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