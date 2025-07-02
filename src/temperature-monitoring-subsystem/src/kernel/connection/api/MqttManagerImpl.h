#ifndef MQTT_MANAGER_IMPL_H
#define MQTT_MANAGER_IMPL_H

#include "MqttManager.h"
#include "WifiManager.h"
#include "config/config.h"
#include <PubSubClient.h>
#include <WiFiClient.h>

/**
 * @class MqttManagerImpl
 * @brief Implements MQTT client functionality using PubSubClient.
 * 
 * Handles connection to an MQTT broker, message publishing, and processing
 * of incoming frequency configuration messages.
 */
class MqttManagerImpl : public MqttManager {
public:
    /**
     * @brief Constructor for MqttManagerImpl.
     * @param host The hostname or IP address of the MQTT broker.
     * @param port The port number of the MQTT broker.
     * @param clientIdPrefix A prefix string used to generate a unique MQTT client ID.
     * @param wifiManager Pointer to a WifiManager instance for checking WiFi status.
     */
    MqttManagerImpl(const char* host, int port, const char* clientIdPrefix, WifiManager* wifiManager);

    /**
     * @brief Virtual destructor.
     */
    virtual ~MqttManagerImpl() {}

    void setup() override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() override;
    void loop() override;
    bool publishTemperature(float temperature) override;
    bool publishStatus(const char* statusMessage) override;
    unsigned long getNewSamplingIntervalMs() override;

private:
    const char* _host;          ///< MQTT broker hostname or IP.
    int _port;                  ///< MQTT broker port.
    String _clientId;           ///< Generated unique client ID.
    WifiManager* _wifiManager;  ///< Pointer to the WiFi manager instance.

    WiFiClient _espClient;      ///< Underlying TCP client for MQTT.
    PubSubClient _mqttClient;   ///< PubSubClient library instance.

    unsigned long _newSamplingInterval; ///< New sampling interval from MQTT.
    bool _newIntervalAvailable;         ///< Flag for new interval availability.

    /**
     * @brief Callback for incoming MQTT messages.
     * @param topic The topic of the received message.
     * @param payload The message payload.
     * @param length The length of the payload.
     */
    void handleMqttMessage(char* topic, byte* payload, unsigned int length);

    /**
     * @brief Static wrapper for C-style callback requirement.
     */
    static void staticMqttCallback(char* topic, byte* payload, unsigned int length);
    
    static MqttManagerImpl* _instance; ///< Static instance pointer for callback.
};

#endif // MQTT_MANAGER_IMPL_H