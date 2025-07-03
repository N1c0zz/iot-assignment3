#include "../api/MqttManagerImpl.h"
#include "../../../config/config.h"
#include <Arduino.h>

// Static instance pointer for callback routing
MqttManagerImpl* MqttManagerImpl::_instance = nullptr;

MqttManagerImpl::MqttManagerImpl(const char* host, int port, const char* clientIdPrefix, WifiManager* wifiManager)
    : _host(host),
      _port(port),
      _wifiManager(wifiManager),
      _mqttClient(_espClient),
      _newSamplingInterval(0),
      _newIntervalAvailable(false) {
    
    _instance = this;

    // Generate unique client ID using ESP32 MAC
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    _clientId = String(clientIdPrefix) + String(chipId, HEX);
}

// Static callback wrapper - required by PubSubClient
void MqttManagerImpl::staticMqttCallback(char* topic, byte* payload, unsigned int length) {
    if (_instance) {
        _instance->handleMqttMessage(topic, payload, length);
    }
}

// Handle incoming MQTT messages
void MqttManagerImpl::handleMqttMessage(char* topic, byte* payload, unsigned int length) {
    // Only handle frequency configuration topic
    if (String(topic) != MQTT_TOPIC_CONFIG_F) {
        return;
    }

    // Convert payload to string
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("MQTT: Frequency config received: ");
    Serial.println(message);

    // JSON parsing look for format "frequency":value
    int freqIndex = message.indexOf("\"frequency\":");
    if (freqIndex >= 0) {
        int valueStart = freqIndex + 12; // Skip past "frequency":
        int valueEnd = message.indexOf(',', valueStart);
        if (valueEnd == -1) valueEnd = message.indexOf('}', valueStart);
        
        if (valueEnd > valueStart) {
            long frequencySeconds = message.substring(valueStart, valueEnd).toInt();
            unsigned long intervalMs = frequencySeconds * 1000;
            
            // Validation for incoming sampling interval
            if (intervalMs >= 1000 && intervalMs <= 600000) {
                _newSamplingInterval = intervalMs;
                _newIntervalAvailable = true;
                Serial.print("MQTT: New sampling interval: ");
                Serial.print(intervalMs);
                Serial.println(" ms");
            } else {
                Serial.println("MQTT: Invalid frequency value");
            }
        }
    }
}

void MqttManagerImpl::setup() {
    _mqttClient.setServer(_host, _port);
    // Set callback function for _mqttClient PubSubClient library instance
    _mqttClient.setCallback(staticMqttCallback); 
    Serial.print("MQTT Manager: Setup completed. Client ID: ");
    Serial.println(_clientId);
}

bool MqttManagerImpl::connect() {
    if (!_wifiManager || !_wifiManager->isConnected()) {
        return false;
    }
    
    if (_mqttClient.connected()) {
        return true;
    }

    Serial.print("MQTT: Connecting to ");
    Serial.print(_host);
    Serial.print(" as ");
    Serial.println(_clientId);

    if (_mqttClient.connect(_clientId.c_str())) {
        Serial.println("MQTT: Connected!");
        
        // Subscribe to frequency topic
        _mqttClient.subscribe(MQTT_TOPIC_CONFIG_F);
        Serial.print("MQTT: Subscribed to ");
        Serial.println(MQTT_TOPIC_CONFIG_F);
        
        // Send online status
        publishStatus("online");
        return true;
    } else {
        Serial.print("MQTT: Connection failed, error: ");
        Serial.println(_mqttClient.state());
        return false;
    }
}

void MqttManagerImpl::disconnect() {
    _mqttClient.disconnect();
    Serial.println("MQTT: Disconnected");
}

bool MqttManagerImpl::isConnected() {
    return _mqttClient.connected();
}

void MqttManagerImpl::loop() {
    if (_wifiManager && _wifiManager->isConnected()) {
        _mqttClient.loop();
    }
}

bool MqttManagerImpl::publishTemperature(float temperature) {
    if (!isConnected()) {
        return false;
    }

    // JSON format: {"temperature":XX.YY}
    String payload = "{\"temperature\":" + String(temperature, 2) + "}";
    
    Serial.print("MQTT: Publishing temperature: ");
    Serial.println(payload);
    
    return _mqttClient.publish(MQTT_TOPIC_TEMPERATURE, payload.c_str(), true);
}

bool MqttManagerImpl::publishStatus(const char* statusMessage) {
    if (!isConnected()) {
        return false;
    }

    // JSON format: {"status":"message"}
    String payload = "{\"status\":\"" + String(statusMessage) + "\"}";
    
    return _mqttClient.publish(MQTT_TOPIC_STATUS, payload.c_str(), true);
}

unsigned long MqttManagerImpl::getNewSamplingIntervalMs() {
    if (_newIntervalAvailable) {
        _newIntervalAvailable = false;
        return _newSamplingInterval;
    }
    return 0;
}