#include "../api/WifiManagerImpl.h"
#include <Arduino.h>

WifiManagerImpl::WifiManagerImpl(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {
    // Constructor set WiFi credentials
}

void WifiManagerImpl::setup() {
    WiFi.mode(WIFI_STA); // Set ESP32 to Station mode (WiFi client)
    Serial.println("WiFi Manager: Setup completed");
}

bool WifiManagerImpl::connect() {
    Serial.print("WiFi: Attempting connection to SSID: '");
    Serial.print(_ssid);
    Serial.println("'");

    // If already connected, don't reconnect
    if (isConnected()) {
        Serial.println("WiFi: Already connected");
        if (WiFi.localIP()) { // Check if IP is valid (not 0.0.0.0)
            Serial.print("WiFi: Current IP address: ");
            Serial.println(WiFi.localIP());
        }
        return true;
    }

    // Start connection attempt
    WiFi.begin(_ssid, _password);

    unsigned long startTime = millis();
    // Wait for connection or timeout
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT_MS) {
            Serial.println("\nWiFi: Connection attempt timed out");
            WiFi.disconnect(); // Ensure disconnection on timeout
            return false;
        }
        delay(500);        // Pause between status checks
        Serial.print("."); // Visual feedback of connection attempt
    }
    
    // Connection successful
    Serial.println("\nWiFi: Connected successfully!");
    Serial.print("WiFi: IP address: ");
    Serial.println(WiFi.localIP());
    
    return true;
}

bool WifiManagerImpl::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

IPAddress WifiManagerImpl::getLocalIP() {
    if (isConnected()) {
        return WiFi.localIP();
    }
    return IPAddress(0, 0, 0, 0); // Return null IP if not connected
}

void WifiManagerImpl::disconnect() {
    WiFi.disconnect(true); // Parameter 'true' also disables WiFi radio
}