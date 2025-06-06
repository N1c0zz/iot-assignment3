#include "../api/MqttManagerImpl.h"
#include "../../../config/config.h"
#include <ArduinoJson.h>
#include <Arduino.h>

// Inizializzazione del puntatore statico all'istanza
MqttManagerImpl *MqttManagerImpl::_instance = nullptr;

MqttManagerImpl::MqttManagerImpl(const char *host, int port, const char *clientIdPrefix, WifiManager *wifiManager)
    : _host(host),
      _port(port),
      _wifiManager(wifiManager), // Salva il puntatore all'interfaccia WiFi
      _mqttClient(_espClient),   // Inizializza il client MQTT con l'istanza WiFiClient
      _newSamplingIntervalReceived(0),
      _newIntervalAvailable(false)
{
    _instance = this; // Necessario per il routing della callback statica

    // Genera un Client ID univoco per la connessione MQTT
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    _actualClientId = String(clientIdPrefix) + String(chipId, HEX);
}

// Router statico per la callback MQTT C-style, chiama il metodo dell'istanza.
void MqttManagerImpl::mqttCallbackRouter(char *topic, byte *payload, unsigned int length)
{
    if (_instance)
    {
        _instance->handleMqttCallback(topic, payload, length);
    }
}

// Gestore effettivo dei messaggi MQTT ricevuti.
void MqttManagerImpl::handleMqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("MQTT: Messaggio ricevuto su topic [");
    Serial.print(topic);
    Serial.print("]: ");

    // Buffer per copiare il payload e assicurare la terminazione null per il parsing JSON.
    char jsonBuffer[256]; // Aumenta se prevedi JSON più grandi
    if (length < sizeof(jsonBuffer) - 1)
    {
        memcpy(jsonBuffer, payload, length);
        jsonBuffer[length] = '\0'; // Null-terminate
        Serial.println(jsonBuffer);
    }
    else
    {
        Serial.println("MQTT Errore: Payload JSON troppo grande per il buffer!");
        return;
    }

    // Controlla se il messaggio è sul topic di configurazione della frequenza.
    if (String(topic) == MQTT_TOPIC_CONFIG_F)
    { // Usa il #define da config.h
        Serial.print("MQTT: Ricevuto comando frequenza (JSON): ");
        Serial.println(jsonBuffer);

        StaticJsonDocument<128> doc; // Dimensione adatta per {"frequency": valore}
        DeserializationError error = deserializeJson(doc, jsonBuffer);

        if (error)
        {
            Serial.print("MQTT: deserializeJson() fallito: ");
            Serial.println(error.f_str());
            return;
        }

        if (doc.containsKey("frequency"))
        {
            long frequency_seconds = doc["frequency"];                    // Estrae il valore in secondi
            unsigned long new_interval_val_ms = frequency_seconds * 1000; // Converte in millisecondi

            Serial.print("MQTT: Frequenza letta (s): ");
            Serial.print(frequency_seconds);
            Serial.print(", Convertita (ms): ");
            Serial.println(new_interval_val_ms);

            // Valida l'intervallo ricevuto (es. tra 1 sec e 10 min).
            if (new_interval_val_ms >= 1000 && new_interval_val_ms <= 600000)
            {
                _newSamplingIntervalReceived = new_interval_val_ms;
                _newIntervalAvailable = true; // Segnala che un nuovo intervallo è pronto
                Serial.print("MQTT: Nuovo intervallo di campionamento valido: ");
                Serial.println(_newSamplingIntervalReceived);
            }
            else
            {
                Serial.println("MQTT: Valore intervallo (ms) non valido.");
            }
        }
        else
        {
            Serial.println("MQTT: Chiave 'frequency' non trovata nel JSON.");
        }
    }
    // Qui potrebbero essere gestiti altri topic in futuro.
}

void MqttManagerImpl::setup()
{
    _mqttClient.setServer(_host, _port);
    _mqttClient.setCallback(mqttCallbackRouter); // Imposta il router statico come callback.
    Serial.print("MQTT: OOP Setup completato. Client ID: ");
    Serial.println(_actualClientId);
    Serial.print("MQTT: Server: ");
    Serial.print(_host);
    Serial.print(":");
    Serial.println(_port);
}

bool MqttManagerImpl::connect()
{
    // Non tentare la connessione MQTT se il WiFi non è pronto.
    if (!_wifiManager || !_wifiManager->isConnected())
    {
        Serial.println("MQTT: Impossibile connettersi, WiFi non pronto.");
        return false;
    }
    if (_mqttClient.connected())
    {
        return true; // Già connesso.
    }

    Serial.print("MQTT: Tentativo di connessione al broker ");
    Serial.print(_host);
    Serial.print(" come ");
    Serial.println(_actualClientId);

    if (_mqttClient.connect(_actualClientId.c_str()))
    {
        Serial.println("MQTT: Connesso!");
        publishStatus("ESP32 OOP Online"); // Invia lo stato online.

        // Sottoscrizione al topic per la configurazione della frequenza.
        Serial.print("MQTT: Sottoscrizione a: ");
        Serial.println(MQTT_TOPIC_CONFIG_F); // Usa il #define da config.h
        if (_mqttClient.subscribe(MQTT_TOPIC_CONFIG_F))
        { // Usa il #define
            Serial.println("MQTT: Sottoscrizione OK.");
        }
        else
        {
            Serial.println("MQTT: Errore sottoscrizione freq.");
        }
        return true;
    }
    else
    {
        Serial.print("MQTT: Connessione fallita, rc=");
        Serial.print(_mqttClient.state()); // Codice di errore PubSubClient.
        Serial.println(" Riproverò...");
        return false;
    }
}

void MqttManagerImpl::disconnect()
{
    _mqttClient.disconnect();
    Serial.println("MQTT: Disconnesso.");
}

bool MqttManagerImpl::isConnected()
{
    return _mqttClient.connected();
}

void MqttManagerImpl::loop()
{
    // Processa il loop MQTT solo se il WiFi è connesso.
    if (_wifiManager && _wifiManager->isConnected())
    {
        _mqttClient.loop(); // Mantiene la connessione e gestisce i messaggi in arrivo.
    }
}

bool MqttManagerImpl::publishTemperature(float temperature)
{
    if (!isConnected())
    {
        Serial.println("MQTT: Impossibile pubblicare temp, non connesso.");
        return false;
    }

    // Crea un documento JSON per la temperatura.
    StaticJsonDocument<100> jsonDoc; // Dimensionato per {"temperature":XX.YY}
    // Arrotonda la temperatura a due cifre decimali per una rappresentazione JSON più pulita.
    jsonDoc["temperature"] = round(temperature * 100.0) / 100.0;

    char jsonBuffer[100];                          // Buffer per la stringa JSON serializzata.
    size_t n = serializeJson(jsonDoc, jsonBuffer); // Serializza e ottiene la lunghezza.

    Serial.print("MQTT: Invio JSON '");
    Serial.print(jsonBuffer);
    Serial.print("' a '");
    Serial.print(MQTT_TOPIC_TEMPERATURE); // Usa il #define da config.h
    Serial.println("'");

    // Pubblica il buffer JSON con la sua lunghezza effettiva. Il flag 'true' lo rende un messaggio "retained".
    if (_mqttClient.publish(MQTT_TOPIC_TEMPERATURE, reinterpret_cast<const uint8_t *>(jsonBuffer), n, true))
    {
        Serial.println("MQTT: Temp JSON inviata.");
        return true;
    }
    else
    {
        Serial.println("MQTT: Errore invio Temp JSON.");
        return false;
    }
}

bool MqttManagerImpl::publishStatus(const char *statusMessage)
{
    if (!isConnected())
    {
        Serial.println("MQTT: Impossibile pubblicare stato, non connesso.");
        return false;
    }

    StaticJsonDocument<100> jsonDoc;
    jsonDoc["status"] = statusMessage;
    char jsonBuffer[100];
    size_t n = serializeJson(jsonDoc, jsonBuffer);

    Serial.print("MQTT: Invio stato '");
    Serial.print(jsonBuffer);
    Serial.print("' a '");
    Serial.print(MQTT_TOPIC_STATUS); // Usa il #define da config.h
    Serial.println("'");

    if (_mqttClient.publish(MQTT_TOPIC_STATUS, reinterpret_cast<const uint8_t *>(jsonBuffer), n, true))
    {
        Serial.println("MQTT: Stato inviato con successo.");
        return true;
    }
    else
    {
        Serial.println("MQTT: Errore invio stato.");
        return false;
    }
}

// Restituisce il nuovo intervallo se disponibile, altrimenti 0.
// Resetta il flag newIntervalAvailable dopo la lettura.
unsigned long MqttManagerImpl::getNewSamplingIntervalMs()
{
    if (_newIntervalAvailable)
    {
        _newIntervalAvailable = false;
        return _newSamplingIntervalReceived;
    }
    return 0;
}

// Metodi di notifica per lo stato del WiFi.
void MqttManagerImpl::onWiFiConnected()
{
    Serial.println("MqttManager: Notifica WiFi Connesso ricevuta.");
    // Potrebbe essere un buon posto per tentare una connessione MQTT se non già connesso.
    // if (!isConnected()) {
    //   connect(); // Tenta la connessione MQTT ora che il WiFi è attivo.
    // }
}

void MqttManagerImpl::onWiFiDisconnected()
{
    Serial.println("MqttManager: Notifica WiFi Disconnesso ricevuta.");
    // Se MQTT era connesso, la libreria PubSubClient dovrebbe gestire la disconnessione
    // quando il suo loop() rileva che il client TCP sottostante non è più valido.
    // Non è strettamente necessario chiamare _mqttClient.disconnect() qui,
    // ma potrebbe aiutare a resettare lo stato interno più rapidamente.
    if (isConnected())
    {
        Serial.println("MQTT: WiFi perso, la connessione MQTT verrà interrotta.");
    }
}