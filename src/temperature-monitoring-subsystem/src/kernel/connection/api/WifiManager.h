#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <IPAddress.h>

/**
 * @class WifiManager
 * @brief Interface for handling WiFi client operations.
 * 
 * Specifies a contract for setting up, connecting to, and managing
 * a WiFi network connection on ESP32 devices.
 */
class WifiManager {
public:
    /**
     * @brief Virtual destructor for proper cleanup.
     */
    virtual ~WifiManager() {}

    /**
     * @brief Initializes WiFi hardware and event handlers.
     * Must be called once during system setup.
     */
    virtual void setup() = 0;

    /**
     * @brief Attempts to connect to the configured WiFi network.
     * @return True if connection is established within the timeout, false otherwise.
     */
    virtual bool connect() = 0;

    /**
     * @brief Checks if the device is currently connected to WiFi.
     * @return True if connected, false otherwise.
     */
    virtual bool isConnected() = 0;

    /**
     * @brief Retrieves the local IP address assigned to the device.
     * @return IPAddress object representing the local IP, or 0.0.0.0 if not connected.
     */
    virtual IPAddress getLocalIP() = 0;

    /**
     * @brief Disconnects from the current WiFi network.
     */
    virtual void disconnect() = 0;
};

#endif // WIFI_MANAGER_H