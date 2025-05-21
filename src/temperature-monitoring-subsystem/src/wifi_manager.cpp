#include "wifi_manager.h"
#include "config.h" // Per WIFI_SSID, WIFI_PASSWORD, WIFI_CONNECT_TIMEOUT_MS
#include <WiFi.h>
#include <Arduino.h> // Per Serial

// CREDENZIALI
const char* WIFI_SSID = "TIM-18202156";
const char* WIFI_PASSWORD = "MbeCOCznKXXVA7j1wzT4tapt";

// Definisci le funzioni di callback per gli eventi WiFi
void WiFiEventStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("\nWiFi: Connesso alla rete WiFi! (Evento)");
}

void WiFiEventStationGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.print("WiFi: Indirizzo IP ottenuto (Evento): ");
  Serial.println(WiFi.localIP());
  // Qui potresti anche impostare un flag o cambiare stato nella FSM se necessario
}

void WiFiEventStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("\nWiFi: Disconnesso dalla rete WiFi. (Evento)");
  Serial.print("Motivo: ");
  Serial.println(info.wifi_sta_disconnected.reason); // Stampa il motivo della disconnessione
  // Qui potresti impostare un flag per la FSM per tentare la riconnessione
  // o cambiare direttamente lo stato a STATE_NETWORK_ERROR se sei in uno stato operativo.
}


void setupWiFi() {
  WiFi.mode(WIFI_STA);
  // Registra le funzioni di callback definite sopra
  WiFi.onEvent(WiFiEventStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiEventStationGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiEventStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  Serial.println("WiFi: Setup con gestione eventi completato.");
}

bool connectWiFi() {
  Serial.print("WiFi: Tentativo di connessione a ");
  Serial.println(WIFI_SSID);
  
  if (isWiFiConnected()) {
      Serial.println("WiFi: Già connesso.");
      // Stampa IP se già connesso, per coerenza con il flusso dell'evento
      if (WiFi.localIP() != IPAddress(0,0,0,0)) { // Assicurati che l'IP sia valido
        Serial.print("WiFi: Indirizzo IP: ");
        Serial.println(WiFi.localIP());
      }
      return true;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > WIFI_CONNECT_TIMEOUT_MS) {
      Serial.println("\nWiFi: Timeout connessione.");
      return false;
    }
    delay(500);
    Serial.print(".");
  }
  // Se arriva qui, è connesso. Gli eventi WiFiEventStationConnected e WiFiEventStationGotIP
  // dovrebbero essere già stati triggerati e aver stampato i loro messaggi.
  // Non c'è bisogno di stampare di nuovo qui per evitare duplicati.
  return true;
}

bool isWiFiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

IPAddress getWiFiLocalIP() {
  return WiFi.localIP();
}

void disconnectWiFi() {
    WiFi.disconnect(true); // true per spegnere anche la radio WiFi
    Serial.println("WiFi: Disconnesso forzatamente.");
}