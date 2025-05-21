#ifndef CONFIG_H
#define CONFIG_H

// --- Configurazioni WiFi ---
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// --- Configurazioni MQTT ---
extern const char* MQTT_SERVER_HOST;
extern const int MQTT_SERVER_PORT;
extern const char* MQTT_CLIENT_ID_PREFIX;
extern const char* MQTT_TOPIC_TEMPERATURE;
extern const char* MQTT_TOPIC_STATUS;
extern const char* MQTT_TOPIC_CONFIG_F;

// --- Configurazioni Sensore ---
#define TEMP_SENSOR_PIN 4 // Tipicamente GPIO1 per A0 su ESP32 Arduino core
                           // VERIFICA IL PINOUT DELLA TUA SCHEDA ESP32-S3-DevKitC-1

// --- Configurazioni LED (Esterni Standard) ---
// VERIFICA I PIN GPIO CHE HAI SCELTO SULLA TUA SCHEDA
#define GREEN_LED_PIN 18  // Esempio, scegli un pin GPIO disponibile
#define RED_LED_PIN   19  // Esempio, scegli un pin GPIO disponibile

// --- Intervalli e Temporizzazioni ---
#define TEMP_SAMPLE_INTERVAL_DEFAULT_MS 10000
#define MQTT_RECONNECT_INTERVAL_MS 5000
#define WIFI_CONNECT_TIMEOUT_MS 15000
#define WIFI_RECONNECT_INTERVAL_MS 10000

#endif // CONFIG_H