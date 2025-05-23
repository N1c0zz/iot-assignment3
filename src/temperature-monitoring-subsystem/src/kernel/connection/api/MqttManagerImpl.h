#ifndef MQTT_MANAGER_IMPL_H
#define MQTT_MANAGER_IMPL_H

#include "MqttManager.h"
#include "WifiManager.h" // Dipendenza
#include "config.h"      // Per i topic e server MQTT
#include <PubSubClient.h>
#include <WiFiClient.h> // Aggiunto per WiFiClient

/**
 * @class MqttManagerImpl
 * @brief Implements MQTT client functionality using PubSubClient.
 * Handles connection to an MQTT broker, message publishing/subscription,
 * and processing of incoming configuration messages (e.g., sampling frequency).
 * Requires an WiFiManager instance for network connectivity.
 */
class MqttManagerImpl : public MqttManager
{
public:
    /**
     * @brief Constructor for MqttManagerImpl.
     * @param host The hostname or IP address of the MQTT broker.
     * @param port The port number of the MQTT broker.
     * @param clientIdPrefix A prefix string used to generate a unique MQTT client ID.
     * @param wifiManager Pointer to an IWiFiManager instance for checking WiFi status.
     */
    MqttManagerImpl(const char *host, int port, const char *clientIdPrefix, WifiManager *wifiManager); // Changed WifiManager* to IWiFiManager*

    /**
     * @brief Default virtual destructor.
     */
    virtual ~MqttManagerImpl() {}

    // Method overrides from IMqttManager
    void setup() override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() override;
    void loop() override;
    bool publishTemperature(float temperature) override;
    bool publishStatus(const char *statusMessage) override;
    unsigned long getNewSamplingIntervalMs() override;
    void onWiFiConnected() override;
    void onWiFiDisconnected() override;

private:
    const char *_host;          ///< MQTT broker hostname or IP.
    int _port;                  ///< MQTT broker port.
    String _actualClientId;     ///< Generated unique client ID for MQTT connection.
    WifiManager *_wifiManager; ///< Pointer to the WiFi manager instance.

    WiFiClient _espClient;    ///< Underlying TCP client for MQTT.
    PubSubClient _mqttClient; ///< PubSubClient library instance.

    /**
     * @brief Static router function for the C-style MQTT callback.
     * Routes the callback to the appropriate class instance's handler method.
     * @param topic The topic of the received message.
     * @param payload The message payload.
     * @param length The length of the payload.
     */
    static void mqttCallbackRouter(char *topic, byte *payload, unsigned int length);

    /**
     * @brief Instance-specific handler for incoming MQTT messages.
     * Processes messages, particularly for frequency configuration.
     * @param topic The topic of the received message.
     * @param payload The message payload.
     * @param length The length of the payload.
     */
    void handleMqttCallback(char *topic, byte *payload, unsigned int length);

    unsigned long _newSamplingIntervalReceived; ///< Stores new sampling interval from MQTT.
    bool _newIntervalAvailable;                 ///< Flag indicating a new interval has been received.

    static MqttManagerImpl *_instance; ///< Static pointer to this instance for C-style callbacks.
};

#endif // MQTT_MANAGER_IMPL_H