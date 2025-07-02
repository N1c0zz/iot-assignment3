#ifndef WIFI_MANAGER_IMPL_H
#define WIFI_MANAGER_IMPL_H

#include "WifiManager.h"
#include "config/config.h"
#include <WiFi.h>

/**
 * @class WifiManagerImpl
 * @brief Implements WiFi client functionality for ESP32.
 * 
 * Handles connecting to a specified WiFi network and monitoring connection status.
 */
class WifiManagerImpl : public WifiManager {
public:
    /**
     * @brief Constructor for WifiManagerImpl.
     * @param ssid The Service Set Identifier (name) of the WiFi network.
     * @param password The password for the WiFi network.
     */
    WifiManagerImpl(const char* ssid, const char* password);

    /**
     * @brief Virtual destructor.
     */
    virtual ~WifiManagerImpl() {}

    void setup() override;
    bool connect() override;
    bool isConnected() override;
    IPAddress getLocalIP() override;
    void disconnect() override;

private:
    const char* _ssid;     ///< SSID of the target WiFi network.
    const char* _password; ///< Password for the target WiFi network.
};

#endif // WIFI_MANAGER_IMPL_H