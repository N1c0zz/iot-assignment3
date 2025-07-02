#ifndef CONFIG_H
#define CONFIG_H

// === WiFi Network Configuration ===
/** @brief SSID of the WiFi network to connect to. */
#define WIFI_SSID "TIM-18202156"
/** @brief Password for the WiFi network. */
#define WIFI_PASSWORD "MbeCOCznKXXVA7j1wzT4tapt"

// === MQTT Broker Configuration ===
/** @brief IP address or hostname of the MQTT broker. */
#define MQTT_SERVER_HOST "192.168.1.100"
/** @brief Port number of the MQTT broker. */
#define MQTT_SERVER_PORT 1883
/** @brief Prefix for generating unique MQTT client IDs. */
#define MQTT_CLIENT_ID_PREFIX "esp32s3-main-mon-"

// === MQTT Topic Configuration ===
/** @brief Topic for publishing temperature data to the Control Unit. */
#define MQTT_TOPIC_TEMPERATURE "assignment3/temperature"
/** @brief Topic for publishing system status messages. */
#define MQTT_TOPIC_STATUS "assignment3/status"
/** @brief Topic for receiving sampling frequency configuration from the Control Unit. */
#define MQTT_TOPIC_CONFIG_F "assignment3/frequency"

// === Hardware Pin Configuration ===
/** @brief Analog GPIO pin connected to the TMP36 temperature sensor. */
#define TEMP_SENSOR_PIN 4
/** @brief Digital GPIO pin for the green status LED. */
#define GREEN_LED_PIN 18
/** @brief Digital GPIO pin for the red status LED. */
#define RED_LED_PIN 19

// === Temperature Sensor Configuration ===
/** @brief ESP32 ADC resolution in bits (12-bit = 4096 levels, 0-4095). */
#define ESP32_ADC_RESOLUTION 4095.0f
/** @brief ESP32 ADC reference voltage in volts. */
#define ESP32_ADC_VREF 3.3f
/** @brief TMP36 voltage-to-temperature conversion factor (mV/°C). */
#define TMP36_MV_PER_CELSIUS 10.0f
/** @brief TMP36 voltage offset at 0°C in mV. */
#define TMP36_OFFSET_MV 500.0f

// === Temperature Validation Limits ===
/** @brief Minimum reasonable temperature for indoor monitoring (°C). */
#define TEMP_MIN_VALID -10.0f
/** @brief Maximum reasonable temperature for indoor monitoring (°C). */
#define TEMP_MAX_VALID 60.0f
/** @brief Delay in milliseconds before re-reading anomalous temperature values. */
#define TEMP_REREAD_DELAY_MS 10

// === Timing and Interval Configuration ===
/** @brief Default temperature sampling interval in milliseconds. */
#define TEMP_SAMPLE_INTERVAL_DEFAULT_MS 10000
/** @brief Interval between MQTT reconnection attempts in milliseconds. */
#define MQTT_RECONNECT_INTERVAL_MS 5000
/** @brief Timeout for WiFi connection attempts in milliseconds. */
#define WIFI_CONNECT_TIMEOUT_MS 15000
/** @brief Interval between WiFi reconnection attempts in milliseconds. */
#define WIFI_RECONNECT_INTERVAL_MS 10000

#endif // CONFIG_H