#include "../api/FsmManagerImpl.h"
#include <Arduino.h> // Per millis(), Serial

FsmManagerImpl::FsmManagerImpl(LedStatus& ledCtrl, TemperatureManager& tempCtrl, WifiManager& wifiCtrl, MqttManager& mqttCtrl)
    : ledController(ledCtrl),
      tempController(tempCtrl),
      wifiController(wifiCtrl),
      mqttController(mqttCtrl),
      _currentState(STATE_INITIALIZING),
      _lastTempSampleTime(0),
      _lastMqttAttemptTime(0),
      _lastWiFiAttemptTime(0),
      _currentTemperature(0.0f),
      _currentSamplingIntervalMs(TEMP_SAMPLE_INTERVAL_DEFAULT_MS) {}

void FsmManagerImpl::setup() {
    // Il setup dei singoli moduli (LED, sensor, WiFi, MQTT)
    // è fatto esternamente in main.cpp prima di creare FsmManagerImpl
    // o potrebbe essere chiamato qui se FsmManagerImpl fosse responsabile della loro creazione.
    // Per ora, assumiamo che siano già stati configurati.

    Serial.println("FSM Impl: Setup. Initial state: INITIALIZING");
    _lastWiFiAttemptTime = millis(); // Inizia il timer per il primo tentativo WiFi
    // L'azione di `indicateSystemBoot()` per i LED è già stata fatta in main.cpp
}

 SystemState FsmManagerImpl::getCurrentState() const {
     return _currentState;
 }

 float FsmManagerImpl::getCurrentTemperature() const {
     return _currentTemperature;
 }

 unsigned long FsmManagerImpl::getCurrentSamplingInterval() const {
     return _currentSamplingIntervalMs;
 }

 void FsmManagerImpl::checkAndUpdateSamplingInterval() {
     unsigned long newInterval = mqttController.getNewSamplingIntervalMs();
     if (newInterval > 0) {
         _currentSamplingIntervalMs = newInterval;
         Serial.print("FSM Impl: Intervallo di campionamento aggiornato a: ");
         Serial.print(_currentSamplingIntervalMs);
         Serial.println(" ms");
     }
 }

void FsmManagerImpl::run() {
    unsigned long currentTime = millis();

    // Il loop MQTT è gestito esternamente in main.cpp,
    // ma le informazioni sullo stato MQTT sono accessibili tramite mqttController.isConnected()

    checkAndUpdateSamplingInterval(); // Controlla sempre se c'è un nuovo intervallo

    switch (_currentState) {
        case STATE_INITIALIZING:
            handleInitializingState();
            break;
        case STATE_WIFI_CONNECTING: // Questo stato è gestito implicitamente da handleInitializingState e dalla logica di connectWiFi
            // Non c'è bisogno di una funzione separata se connectWiFi è bloccante (con timeout)
            // Se connectWiFi fosse non bloccante, qui si controllerebbe lo stato.
            // Attualmente, handleInitializingState transita direttamente o a WIFI_CONNECTED o a NETWORK_ERROR.
            break;
        case STATE_WIFI_CONNECTED:
            handleWiFiConnectedState();
            break;
        case STATE_MQTT_CONNECTING:
            handleMqttConnectingState(currentTime);
            break;
        case STATE_OPERATIONAL:
            handleOperationalState(currentTime);
            break;
        case STATE_SAMPLING_TEMPERATURE:
            handleSamplingTemperatureState(currentTime);
            break;
        case STATE_SENDING_DATA:
            handleSendingDataState();
            break;
        case STATE_NETWORK_ERROR:
            handleNetworkErrorState(currentTime);
            break;
        case STATE_WAIT_RECONNECT:
            handleWaitReconnectState(currentTime);
            break;
        default:
            Serial.println("FSM Impl: Stato sconosciuto! Ritorno a INITIALIZING.");
            ledController.indicateNetworkError();
            _currentState = STATE_INITIALIZING;
            break;
    }
}

// Implementazioni dei metodi privati per ogni stato
void FsmManagerImpl::handleInitializingState() {
    Serial.println("FSM Impl: STATE_INITIALIZING -> Tentativo connessione WiFi");
    ledController.indicateWifiConnecting();
    if (wifiController.connect()) { // connect() ora è membro di WifiManager
        _currentState = STATE_WIFI_CONNECTED;
        Serial.println("FSM Impl: WiFi Connesso -> STATE_WIFI_CONNECTED");
        mqttController.onWiFiConnected();
    } else {
        Serial.println("FSM Impl: WiFi Connessione iniziale fallita -> STATE_NETWORK_ERROR");
        ledController.indicateNetworkError();
        _currentState = STATE_NETWORK_ERROR;
        _lastWiFiAttemptTime = millis();
        mqttController.onWiFiDisconnected();
    }
}

void FsmManagerImpl::handleWiFiConnectedState() {
    Serial.println("FSM Impl: STATE_WIFI_CONNECTED -> Tentativo connessione MQTT");
    ledController.indicateMqttConnecting();
    _currentState = STATE_MQTT_CONNECTING;
    _lastMqttAttemptTime = 0; // Per forzare un tentativo MQTT immediato.
}

void FsmManagerImpl::handleMqttConnectingState(unsigned long currentTime) {
    if (mqttController.isConnected()) {
        Serial.println("FSM Impl: MQTT Connesso -> STATE_OPERATIONAL");
        ledController.indicateOperational();
        _currentState = STATE_OPERATIONAL;
    } else if (currentTime - _lastMqttAttemptTime >= MQTT_RECONNECT_INTERVAL_MS) {
        Serial.println("FSM Impl: Riprovo connessione MQTT...");
        ledController.indicateMqttConnecting();
        if (wifiController.isConnected()) {
            if (!mqttController.connect()) {
                Serial.println("FSM Impl: Tentativo MQTT fallito, attendo prossimo intervallo.");
            }
        } else {
            Serial.println("FSM Impl: WiFi perso prima di tentare MQTT.");
        }
        _lastMqttAttemptTime = currentTime;
    }

    if (!wifiController.isConnected()) {
        Serial.println("FSM Impl: WiFi perso durante tentativo MQTT -> STATE_NETWORK_ERROR");
        ledController.indicateNetworkError();
        _currentState = STATE_NETWORK_ERROR;
        _lastWiFiAttemptTime = currentTime;
        mqttController.onWiFiDisconnected();
    }
}

void FsmManagerImpl::handleOperationalState(unsigned long currentTime) {
    ledController.indicateOperational();
    if (!wifiController.isConnected() || !mqttController.isConnected()) {
        Serial.println("FSM Impl: Connessione persa (WiFi o MQTT) -> STATE_NETWORK_ERROR");
        ledController.indicateNetworkError();
        if (!wifiController.isConnected()) mqttController.onWiFiDisconnected();
        _currentState = STATE_NETWORK_ERROR;
        _lastWiFiAttemptTime = currentTime;
        return;
    }
    if (currentTime - _lastTempSampleTime >= _currentSamplingIntervalMs) {
        _currentState = STATE_SAMPLING_TEMPERATURE;
        Serial.println("FSM Impl: -> STATE_SAMPLING_TEMPERATURE");
    }
}

void FsmManagerImpl::handleSamplingTemperatureState(unsigned long currentTime) {
    Serial.println("FSM Impl: Campionamento temperatura...");
    _currentTemperature = tempController.readTemperature();
    Serial.print("FSM Impl: Temperatura: ");
    Serial.print(_currentTemperature);
    Serial.println(" C");
    _lastTempSampleTime = currentTime;
    _currentState = STATE_SENDING_DATA;
    Serial.println("FSM Impl: -> STATE_SENDING_DATA");
}

void FsmManagerImpl::handleSendingDataState() {
    Serial.println("FSM Impl: Invio dati temperatura...");
    if (mqttController.publishTemperature(_currentTemperature)) {
        Serial.println("FSM Impl: Dati inviati.");
    } else {
        Serial.println("FSM Impl: Fallito invio dati. MQTT potrebbe essere disconnesso.");
    }
    _currentState = STATE_OPERATIONAL;
    Serial.println("FSM Impl: -> STATE_OPERATIONAL (dopo invio)");
}

void FsmManagerImpl::handleNetworkErrorState(unsigned long currentTime) {
    ledController.indicateNetworkError();
    Serial.println("FSM Impl: Errore di rete. In attesa per ritentare...");
    if (mqttController.isConnected()) mqttController.disconnect();
    _currentState = STATE_WAIT_RECONNECT;
    _lastWiFiAttemptTime = currentTime; // Usa _lastWiFiAttemptTime qui
    Serial.println("FSM Impl: -> STATE_WAIT_RECONNECT");
}

void FsmManagerImpl::handleWaitReconnectState(unsigned long currentTime) {
    ledController.indicateNetworkError();
    if (currentTime - _lastWiFiAttemptTime >= WIFI_RECONNECT_INTERVAL_MS) {
        Serial.println("FSM Impl: Fine attesa, ritento connessione (-> INITIALIZING)...");
        _currentState = STATE_INITIALIZING;
    }
}