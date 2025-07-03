#include "../api/FsmManagerImpl.h"
#include <Arduino.h>

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
    Serial.println("FSM Manager: Setup. Initial state: INITIALIZING");
    _lastWiFiAttemptTime = millis(); // Initialize timer for first WiFi attempt
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
    }
}

void FsmManagerImpl::run() {
    unsigned long currentTime = millis();

    // Check for new sampling interval configuration
    checkAndUpdateSamplingInterval();

    switch (_currentState) {
        case STATE_INITIALIZING:
            handleInitializingState();
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
            Serial.println("FSM Manager: Unknown state. Returning to INITIALIZING.");
            ledController.indicateNetworkError();
            _currentState = STATE_INITIALIZING;
            break;
    }
}

void FsmManagerImpl::handleInitializingState() {
    Serial.println("FSM Manager: STATE_INITIALIZING -> Attempting WiFi connection");
    ledController.indicateWifiConnecting();
    
    if (wifiController.connect()) {
        _currentState = STATE_WIFI_CONNECTED;
        Serial.println("FSM Manager: WiFi Connected -> STATE_WIFI_CONNECTED");
    } else {
        Serial.println("FSM Manager: WiFi connection failed -> STATE_NETWORK_ERROR");
        ledController.indicateNetworkError();
        _currentState = STATE_NETWORK_ERROR;
        _lastWiFiAttemptTime = millis();
    }
}

void FsmManagerImpl::handleWiFiConnectedState() {
    Serial.println("FSM Manager: STATE_WIFI_CONNECTED -> Attempting MQTT connection");
    ledController.indicateMqttConnecting();
    _currentState = STATE_MQTT_CONNECTING;
    _lastMqttAttemptTime = 0; // Force immediate MQTT attempt
}

void FsmManagerImpl::handleMqttConnectingState(unsigned long currentTime) {
    if (mqttController.isConnected()) {
        Serial.println("FSM Manager: MQTT Connected -> STATE_OPERATIONAL");
        mqttController.publishStatus("online");
        ledController.indicateOperational();
        _currentState = STATE_OPERATIONAL;
    } else if (currentTime - _lastMqttAttemptTime >= MQTT_RECONNECT_INTERVAL_MS) {
        Serial.println("FSM Manager: Retrying MQTT connection...");
        ledController.indicateMqttConnecting();
        
        if (wifiController.isConnected()) {
            if (!mqttController.connect()) {
                Serial.println("FSM Manager: MQTT attempt failed, waiting for next interval.");
            }
        } else {
            Serial.println("FSM Manager: WiFi lost before MQTT attempt.");
        }
        _lastMqttAttemptTime = currentTime;
    }

    // Check if WiFi was lost during MQTT connection attempts
    if (!wifiController.isConnected()) {
        Serial.println("FSM Manager: WiFi lost during MQTT attempt -> STATE_NETWORK_ERROR");
        ledController.indicateNetworkError();
        _currentState = STATE_NETWORK_ERROR;
        _lastWiFiAttemptTime = currentTime;
    }
}

void FsmManagerImpl::handleOperationalState(unsigned long currentTime) {
    ledController.indicateOperational();
    
    // Check for connection loss
    if (!wifiController.isConnected() || !mqttController.isConnected()) {
        Serial.println("FSM Manager: Connection lost (WiFi or MQTT) -> STATE_NETWORK_ERROR");
        ledController.indicateNetworkError();
        _currentState = STATE_NETWORK_ERROR;
        _lastWiFiAttemptTime = currentTime;
        return;
    }

    // Check if it's time to sample temperature
    if (currentTime - _lastTempSampleTime >= _currentSamplingIntervalMs) {
        _currentState = STATE_SAMPLING_TEMPERATURE;
        Serial.println("FSM Manager: -> STATE_SAMPLING_TEMPERATURE");
    }
}

void FsmManagerImpl::handleSamplingTemperatureState(unsigned long currentTime) {
    Serial.println("FSM Manager: Sampling temperature...");
    _currentTemperature = tempController.readTemperature();
    Serial.print("FSM Manager: Temperature: ");
    Serial.print(_currentTemperature);
    Serial.println(" °C");
    
    _lastTempSampleTime = currentTime;
    _currentState = STATE_SENDING_DATA;
    Serial.println("FSM Manager: -> STATE_SENDING_DATA");
}

void FsmManagerImpl::handleSendingDataState() {
    Serial.println("FSM Manager: Sending temperature data...");
    
    if (mqttController.publishTemperature(_currentTemperature)) {
        Serial.println("FSM Manager: Data sent successfully.");
    } else {
        Serial.println("FSM Manager: Failed to send data. MQTT may be disconnected.");
    }
    
    _currentState = STATE_OPERATIONAL;
    Serial.println("FSM Manager: -> STATE_OPERATIONAL (after sending)");
}

void FsmManagerImpl::handleNetworkErrorState(unsigned long currentTime) {
    ledController.indicateNetworkError();
    Serial.println("FSM Manager: Network error. Waiting before retry...");
    
    if (mqttController.isConnected()) {
        mqttController.disconnect();
    }
    
    _currentState = STATE_WAIT_RECONNECT;
    _lastWiFiAttemptTime = currentTime;
    Serial.println("FSM Manager: -> STATE_WAIT_RECONNECT");
}

void FsmManagerImpl::handleWaitReconnectState(unsigned long currentTime) {
    ledController.indicateNetworkError();
    
    if (currentTime - _lastWiFiAttemptTime >= WIFI_RECONNECT_INTERVAL_MS) {
        Serial.println("FSM Manager: Wait period over, retrying connection (-> INITIALIZING)...");
        _currentState = STATE_INITIALIZING;
    }
}