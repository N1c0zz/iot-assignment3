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
// void setMqttCallback(MQTT_CALLBACK_SIGNATURE); // Per futura implementazione

#endif // MQTT_MANAGER_H