#ifndef WIFI_MANAGER_IMPL_H
#define WIFI_MANAGER_IMPL_H

#include "WiFiManager.h"
#include "config/config.h" // Per timeout, ma SSID/PASS saranno passati
#include <WiFi.h>          // Necessario per WiFiEventInfo_t

/**
 * @class WiFiManagerImpl
 * @brief Implements WiFi client functionality for ESP32.
 * Handles connecting to a specified WiFi network and monitoring connection status
 * using ESP32 WiFi events.
 */
class WifiManagerImpl : public WifiManager
{
public:
    /**
     * @brief Constructor for WiFiManagerImpl.
     * @param ssid The Service Set Identifier (name) of the WiFi network.
     * @param password The password for the WiFi network.
     */
    WifiManagerImpl(const char *ssid, const char *password);

    /**
     * @brief Default virtual destructor.
     */
    virtual ~WifiManagerImpl() {}

    // Method overrides from WifiManager
    void setup() override;
    bool connect() override;
    bool isConnected() override;
    IPAddress getLocalIP() override;
    void disconnect() override;

private:
    const char *_ssid;     ///< SSID of the target WiFi network.
    const char *_password; ///< Password for the target WiFi network.

    // Static callback handlers for ESP32 WiFi events.
    // These are static because C-style function pointers are required by WiFi.onEvent.
    static void WiFiEventStationConnectedCb(WiFiEvent_t event, WiFiEventInfo_t info);
    static void WiFiEventStationGotIPCb(WiFiEvent_t event, WiFiEventInfo_t info);
    static void WiFiEventStationDisconnectedCb(WiFiEvent_t event, WiFiEventInfo_t info);
};

#endif // WIFI_MANAGER_IMPL_H