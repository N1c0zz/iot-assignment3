#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>

/**
 * @class MqttManager
 * @brief Interface for handling MQTT client operations.
 * 
 * Defines a contract for connecting to an MQTT broker, publishing messages,
 * and receiving configuration updates. Manages communication between 
 * the ESP32 and the Control Unit via MQTT.
 */
class MqttManager {
public:
    /**
     * @brief Virtual destructor for proper cleanup.
     */
    virtual ~MqttManager() {}

    /**
     * @brief Initializes MQTT client settings.
     * Must be called once during system setup.
     */
    virtual void setup() = 0;

    /**
     * @brief Attempts to connect to the configured MQTT broker.
     * @return True if connection is successful, false otherwise.
     */
    virtual bool connect() = 0;

    /**
     * @brief Disconnects from the MQTT broker.
     */
    virtual void disconnect() = 0;

    /**
     * @brief Checks if the client is currently connected to the MQTT broker.
     * @return True if connected, false otherwise.
     */
    virtual bool isConnected() = 0;

    /**
     * @brief Maintains the MQTT connection and processes incoming messages.
     * Must be called regularly in the main system loop.
     */
    virtual void loop() = 0;

    /**
     * @brief Publishes the current temperature value.
     * @param temperature The temperature value to publish.
     * @return True if publishing was successful, false otherwise.
     */
    virtual bool publishTemperature(float temperature) = 0;

    /**
     * @brief Publishes a status message.
     * @param statusMessage The status message string to publish.
     * @return True if publishing was successful, false otherwise.
     */
    virtual bool publishStatus(const char* statusMessage) = 0;

    /**
     * @brief Retrieves a new sampling interval if received via MQTT.
     * @return The new sampling interval in milliseconds if available, otherwise 0.
     */
    virtual unsigned long getNewSamplingIntervalMs() = 0;
};

#endif // MQTT_MANAGER_H