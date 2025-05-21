#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <IPAddress.h> // Per IPAddress

void setupWiFi();
bool connectWiFi(); // Tenta di connettersi, ritorna true se connesso subito
bool isWiFiConnected();
IPAddress getWiFiLocalIP();
void disconnectWiFi(); // Aggiunta per forzare la disconnessione

#endif // WIFI_MANAGER_H