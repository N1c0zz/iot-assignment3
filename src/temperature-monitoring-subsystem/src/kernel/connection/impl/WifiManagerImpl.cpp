#include "../api/WifiManagerImpl.h"
#include <Arduino.h> // Per Serial, delay, millis

// Definizioni delle callback statiche per gli eventi WiFi.
// Queste funzioni sono chiamate dal sistema WiFi dell'ESP32.
void WifiManagerImpl::WiFiEventStationConnectedCb(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("\nWiFi: Connesso alla rete WiFi! (Evento)");
}

void WifiManagerImpl::WiFiEventStationGotIPCb(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("WiFi: Indirizzo IP ottenuto (Evento): ");
    Serial.println(WiFi.localIP());
    // Questo è un buon punto per notificare ad altri moduli (come MqttManager)
    // che l'IP è disponibile e la connessione è pronta.
}

void WifiManagerImpl::WiFiEventStationDisconnectedCb(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("\nWiFi: Disconnesso dalla rete WiFi. (Evento)");
    Serial.print("Motivo della disconnessione: ");
    // Stampa il codice del motivo per aiutare nel debug.
    // I codici sono definiti in wifi_err_reason_t in esp_wifi_types.h.
    Serial.println(info.wifi_sta_disconnected.reason);
}

WifiManagerImpl::WifiManagerImpl(const char *ssid, const char *password)
    : _ssid(ssid), _password(password)
{
    // Il costruttore memorizza le credenziali WiFi.
}

void WifiManagerImpl::setup()
{
    WiFi.mode(WIFI_STA); // Imposta l'ESP32 in modalità Station (client WiFi).

    // Registra le funzioni di callback statiche per gli eventi WiFi.
    WiFi.onEvent(WiFiEventStationConnectedCb, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiEventStationGotIPCb, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiEventStationDisconnectedCb, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    Serial.println("WiFi: OOP Setup con gestione eventi completato.");
}

bool WifiManagerImpl::connect()
{
    Serial.print("WiFi: Tentativo di connessione a SSID: '");
    Serial.print(_ssid);
    Serial.println("'");

    // Se già connesso, non fare nulla e restituisci true.
    if (isConnected())
    {
        Serial.println("WiFi: Già connesso.");
        if (WiFi.localIP())
        { // Controlla se l'IP è valido (non 0.0.0.0)
            Serial.print("WiFi: Indirizzo IP corrente: ");
            Serial.println(WiFi.localIP());
        }
        return true;
    }

    // Avvia il tentativo di connessione.
    WiFi.begin(_ssid, _password);

    unsigned long startTime = millis();
    // Attendi la connessione o il timeout.
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT_MS)
        { // Timeout da config.h
            Serial.println("\nWiFi: Timeout durante il tentativo di connessione.");
            WiFi.disconnect(); // Assicurati di disconnettere in caso di timeout.
            return false;
        }
        delay(500);        // Pausa tra i controlli di stato.
        Serial.print("."); // Feedback visivo del tentativo.
    }
    // Se il loop esce, significa che WiFi.status() == WL_CONNECTED.
    // I messaggi di "Connesso" e "IP Ottenuto" verranno stampati dalle callback degli eventi.
    return true;
}

bool WifiManagerImpl::isConnected()
{
    return (WiFi.status() == WL_CONNECTED);
}

IPAddress WifiManagerImpl::getLocalIP()
{
    if (isConnected())
    {
        return WiFi.localIP();
    }
    return IPAddress(0, 0, 0, 0); // Restituisce un IP nullo se non connesso.
}

void WifiManagerImpl::disconnect()
{
    WiFi.disconnect(true); // Il parametro 'true' disattiva anche la radio WiFi.
    Serial.println("WiFi: Disconnesso forzatamente.");
}