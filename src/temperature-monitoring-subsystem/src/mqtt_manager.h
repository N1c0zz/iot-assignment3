#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h> // Per String

void setupMQTT();
bool connectMQTT();
void disconnectMQTT();
bool isMQTTConnected();
void mqttLoop(); // Deve essere chiamata regolarmente
bool publishTemperature(float temperature);
bool publishStatus(const char* statusMessage);
// Funzione per ottenere il nuovo intervallo di campionamento ricevuto via MQTT
// Ritorna 0 se nessun nuovo intervallo è stato ricevuto dall'ultima chiamata,
// altrimenti ritorna il nuovo intervallo.
unsigned long getNewSamplingIntervalMs();

#endif // MQTT_MANAGER_H