#ifndef CONFIG_H
#define CONFIG_H

// === DEFINIZIONE DELLE CONFIGURAZIONI SPECIFICHE ===
#define WIFI_SSID "TIM-18202156"
#define WIFI_PASSWORD "MbeCOCznKXXVA7j1wzT4tapt"
#define MQTT_SERVER_HOST "192.168.1.100"
#define MQTT_SERVER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "esp32s3-main-mon-"
#define MQTT_TOPIC_TEMPERATURE "assignment3/temperature"
#define MQTT_TOPIC_STATUS "assignment3/status"
#define MQTT_TOPIC_CONFIG_F "assignment3/frequency"

// --- Configurazioni Sensore ---
#define TEMP_SENSOR_PIN 4

// --- Configurazioni LED ---
#define GREEN_LED_PIN 18
#define RED_LED_PIN   19

// --- Intervalli e Temporizzazioni ---
#define TEMP_SAMPLE_INTERVAL_DEFAULT_MS 10000
#define MQTT_RECONNECT_INTERVAL_MS 5000
#define WIFI_CONNECT_TIMEOUT_MS 15000
#define WIFI_RECONNECT_INTERVAL_MS 10000

#endif // CONFIG_H