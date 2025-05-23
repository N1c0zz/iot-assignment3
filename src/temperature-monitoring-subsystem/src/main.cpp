#include <Arduino.h>
#include "config/config.h"     // Per le costanti globali e intervalli di default
#include "kernel/FsmManager.h" // Per SystemState enum

// Includi le INTERFACCE
#include "devices/api/LedStatus.h"
#include "devices/api/TemperatureManager.h"
#include "kernel/connection/api/WifiManager.h"
#include "kernel/connection/api/MqttManager.h"

// Includi le IMPLEMENTAZIONI (per poter creare le istanze)
#include "devices/api/LedStatusImpl.h"
#include "devices/api/TemperatureManagerImpl.h"
#include "kernel/connection/api/WifiManagerImpl.h"
#include "kernel/connection/api/MqttManagerImpl.h"

// === Puntatori alle interfacce per i moduli del sistema ===
// Utilizzare puntatori alle interfacce promuove il disaccoppiamento.
LedStatus *ledStatus = nullptr;                   ///< Pointer to the LedStatus interface implementation.
TemperatureManager *temperatureManager = nullptr; ///< Pointer to the TemperatureManager interface implementation.
WifiManager *wifiManager = nullptr;               ///< Pointer to the WifiManager interface implementation.
MqttManager *mqttManager = nullptr;               ///< Pointer to the MqttManager interface implementation.

// === Variabili Globali della FSM e Temporizzazione ===
SystemState currentState = STATE_INITIALIZING;                             ///< Global variable holding the current FSM state. Defined in FsmManager.h.
unsigned long lastTempSampleTime = 0;                                      ///< Timestamp of the last temperature sample.
unsigned long lastMqttAttemptTime = 0;                                     ///< Timestamp of the last MQTT connection attempt.
unsigned long lastWiFiAttemptTime = 0;                                     ///< Timestamp of the last WiFi connection attempt.
float currentTemperature = 0.0;                                            ///< Stores the most recently read temperature.
unsigned long currentSamplingIntervalMs = TEMP_SAMPLE_INTERVAL_DEFAULT_MS; ///< Current interval for temperature sampling, can be updated via MQTT.

/**
 * @brief Setup function, runs once at system startup.
 * Initializes serial communication, module instances (LED, Sensor, WiFi, MQTT),
 * and sets the initial state of the FSM.
 */
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }
  Serial.println("\n\n[Smart Temp Monitor - ESP32 OOP] Avvio Sistema...");

  // Crea istanze delle implementazioni concrete e le assegna ai puntatori delle interfacce.
  // I PIN dei LED e del sensore sono presi dai #define in config.h, usati dai costruttori di default.
  ledStatus = new LedStatusImpl();
  temperatureManager = new TemperatureManagerImpl();
  wifiManager = new WifiManagerImpl(WIFI_SSID, WIFI_PASSWORD);
  mqttManager = new MqttManagerImpl(MQTT_SERVER_HOST, MQTT_SERVER_PORT, MQTT_CLIENT_ID_PREFIX, wifiManager);

  // Esegui il setup per ogni modulo.
  ledStatus->setup();
  ledStatus->indicateSystemBoot(); // Feedback visivo immediato dell'avvio.

  temperatureManager->setup();
  wifiManager->setup(); // Registra gli event handler interni del WiFi.
  mqttManager->setup(); // Configura il client MQTT (server, porta, callback).

  Serial.println("Setup OOP iniziale completato.");
  currentState = STATE_INITIALIZING; // Imposta lo stato iniziale della FSM.
  lastWiFiAttemptTime = millis();    // Inizia il timer per il primo tentativo di connessione WiFi.
}

/**
 * @brief Main loop function, runs repeatedly.
 * Manages MQTT communication, checks for FSM state transitions,
 * and executes actions based on the current state.
 */
void loop()
{
  unsigned long currentTime = millis(); // Ottieni il timestamp corrente per la logica temporizzata.

  // Il loop MQTT deve essere chiamato regolarmente per mantenere la connessione
  // e processare i messaggi in entrata/uscita.
  if (wifiManager && wifiManager->isConnected())
  { // Esegui solo se wifiManager è inizializzato e connesso
    if (mqttManager)
    { // Esegui solo se mqttManager è inizializzato
      mqttManager->loop();
    }
  }

  // Controlla se è stato ricevuto un nuovo intervallo di campionamento via MQTT.
  if (mqttManager)
  { // Esegui solo se mqttManager è inizializzato
    unsigned long newInterval = mqttManager->getNewSamplingIntervalMs();
    if (newInterval > 0)
    { // Un valore > 0 indica un nuovo intervallo valido ricevuto.
      currentSamplingIntervalMs = newInterval;
      Serial.print("Main: Intervallo di campionamento aggiornato a: ");
      Serial.print(currentSamplingIntervalMs);
      Serial.println(" ms");
      // Considerare se resettare lastTempSampleTime qui per applicare subito il nuovo intervallo.
    }
  }

  // Gestione della Macchina a Stati Finiti (FSM).
  switch (currentState)
  {
  case STATE_INITIALIZING:
    /** @details Inizializza la connessione WiFi. Passa a WIFI_CONNECTED o NETWORK_ERROR. */
    Serial.println("FSM: STATE_INITIALIZING -> Tentativo connessione WiFi");
    ledStatus->indicateWifiConnecting();
    if (wifiManager->connect())
    {
      currentState = STATE_WIFI_CONNECTED;
      Serial.println("FSM: WiFi Connesso -> STATE_WIFI_CONNECTED");
      if (mqttManager)
        mqttManager->onWiFiConnected(); // Notifica il gestore MQTT.
    }
    else
    {
      Serial.println("FSM: WiFi Connessione iniziale fallita -> STATE_NETWORK_ERROR");
      ledStatus->indicateNetworkError();
      currentState = STATE_NETWORK_ERROR;
      lastWiFiAttemptTime = currentTime;
      if (mqttManager)
        mqttManager->onWiFiDisconnected(); // Notifica il gestore MQTT.
    }
    break;

  case STATE_WIFI_CONNECTED:
    /** @details Stato transitorio post-connessione WiFi. Inizia la connessione MQTT. */
    Serial.println("FSM: STATE_WIFI_CONNECTED -> Tentativo connessione MQTT");
    ledStatus->indicateMqttConnecting();
    currentState = STATE_MQTT_CONNECTING;
    lastMqttAttemptTime = 0; // Per forzare un tentativo MQTT immediato.
    break;

  case STATE_MQTT_CONNECTING:
    /** @details Tenta di connettersi al broker MQTT. Passa a OPERATIONAL o NETWORK_ERROR. */
    if (mqttManager->isConnected())
    {
      Serial.println("FSM: MQTT Connesso -> STATE_OPERATIONAL");
      ledStatus->indicateOperational();
      currentState = STATE_OPERATIONAL;
    }
    else if (currentTime - lastMqttAttemptTime >= MQTT_RECONNECT_INTERVAL_MS)
    {
      Serial.println("FSM: Riprovo connessione MQTT...");
      ledStatus->indicateMqttConnecting();
      if (wifiManager->isConnected())
      {
        if (!mqttManager->connect())
        { // Tenta la connessione.
          Serial.println("FSM: Tentativo MQTT fallito, attendo prossimo intervallo.");
        }
      }
      else
      {
        Serial.println("FSM: WiFi perso prima di tentare MQTT.");
        // La FSM transiterà a NETWORK_ERROR nel prossimo controllo.
      }
      lastMqttAttemptTime = currentTime;
    }
    // Controllo critico: se il WiFi è caduto mentre si tentava MQTT.
    if (!wifiManager->isConnected())
    {
      Serial.println("FSM: WiFi perso durante tentativo MQTT -> STATE_NETWORK_ERROR");
      ledStatus->indicateNetworkError();
      currentState = STATE_NETWORK_ERROR;
      lastWiFiAttemptTime = currentTime;
      if (mqttManager)
        mqttManager->onWiFiDisconnected();
    }
    break;

  case STATE_OPERATIONAL:
    /** @details Stato principale di funzionamento. Monitora connessioni e avvia campionamento. */
    ledStatus->indicateOperational();
    // Verifica la stabilità delle connessioni.
    if (!wifiManager->isConnected() || !mqttManager->isConnected())
    {
      Serial.println("FSM: Connessione persa (WiFi o MQTT) -> STATE_NETWORK_ERROR");
      ledStatus->indicateNetworkError();
      if (!wifiManager->isConnected() && mqttManager)
        mqttManager->onWiFiDisconnected();
      currentState = STATE_NETWORK_ERROR;
      lastWiFiAttemptTime = currentTime;
      break; // Esce dallo switch per questo ciclo.
    }
    // Se è ora di campionare la temperatura.
    if (currentTime - lastTempSampleTime >= currentSamplingIntervalMs)
    {
      currentState = STATE_SAMPLING_TEMPERATURE;
      Serial.println("FSM: -> STATE_SAMPLING_TEMPERATURE");
    }
    break;

  case STATE_SAMPLING_TEMPERATURE:
    /** @details Legge il valore corrente dal sensore di temperatura. */
    Serial.println("FSM: Campionamento temperatura...");
    currentTemperature = temperatureManager->readTemperature();
    Serial.print("Temperatura: ");
    Serial.print(currentTemperature);
    Serial.println(" C");
    lastTempSampleTime = currentTime;
    currentState = STATE_SENDING_DATA;
    Serial.println("FSM: -> STATE_SENDING_DATA");
    break;

  case STATE_SENDING_DATA:
    /** @details Invia i dati di temperatura via MQTT. */
    Serial.println("FSM: Invio dati temperatura...");
    if (mqttManager->publishTemperature(currentTemperature))
    {
      Serial.println("FSM: Dati inviati.");
    }
    else
    {
      Serial.println("FSM: Fallito invio dati. MQTT potrebbe essere disconnesso.");
      // La disconnessione MQTT verrà gestita in STATE_OPERATIONAL.
    }
    currentState = STATE_OPERATIONAL; // Ritorna allo stato operativo.
    Serial.println("FSM: -> STATE_OPERATIONAL (dopo invio)");
    break;

  case STATE_NETWORK_ERROR:
    /** @details Gestisce errori di rete. Avvia la sequenza di riconnessione. */
    ledStatus->indicateNetworkError();
    Serial.println("FSM: Errore di rete. In attesa per ritentare connessione generale...");
    if (mqttManager && mqttManager->isConnected())
      mqttManager->disconnect(); // Tenta una disconnessione pulita MQTT.
    // Non è necessario disconnettere il WiFi qui, il ciclo di reinizializzazione lo gestirà.
    currentState = STATE_WAIT_RECONNECT;
    Serial.println("FSM: -> STATE_WAIT_RECONNECT");
    lastWiFiAttemptTime = currentTime; // Inizia il timer per l'attesa.
    break;

  case STATE_WAIT_RECONNECT:
    /** @details Periodo di attesa dopo un errore di rete prima di riprovare. */
    ledStatus->indicateNetworkError(); // Mantiene l'indicazione di errore.
    if (currentTime - lastWiFiAttemptTime >= WIFI_RECONNECT_INTERVAL_MS)
    {
      Serial.println("FSM: Fine attesa, ritento connessione WiFi (tornando a INITIALIZING)...");
      currentState = STATE_INITIALIZING; // Riavvia il processo di connessione.
    }
    break;

  default:
    /** @details Gestisce stati FSM imprevisti o non validi. */
    Serial.println("FSM: Stato sconosciuto! Ritorno a INITIALIZING.");
    ledStatus->indicateNetworkError(); // Segnala un problema.
    currentState = STATE_INITIALIZING; // Torna a uno stato sicuro.
    break;
  }
  delay(100); // Breve delay per stabilità e per dare tempo ai processi in background dell'ESP32.
}