#include "../api/TemperatureManagerImpl.h"
#include <Arduino.h>

TemperatureManagerImpl::TemperatureManagerImpl(int sensorPin) : _sensorPin(sensorPin) {
    // Constructor initializes the sensor pin.
    // Hardware pin configuration is performed in setup().
}

void TemperatureManagerImpl::setup() {
    pinMode(_sensorPin, INPUT);
}

float TemperatureManagerImpl::readTemperature() {
    // Read raw value from ESP32 ADC
    int sensorValue = analogRead(_sensorPin);
    
    // Convert ADC value to voltage
    float voltage = (float)sensorValue / ESP32_ADC_RESOLUTION * ESP32_ADC_VREF;
    
    // Convert voltage to Celsius according to TMP36 specifications
    // Formula: Temperature(°C) = (Voltage(mV) - Offset) / Sensitivity
    // TMP36: 10mV/°C sensitivity with 500mV offset at 0°C
    float voltageMillivolts = voltage * 1000.0f; // Convert to mV
    float temperatureC = (voltageMillivolts - TMP36_OFFSET_MV) / TMP36_MV_PER_CELSIUS;
    
    // Filter for anomalous values - validate against expected indoor range
    if (temperatureC < TEMP_MIN_VALID || temperatureC > TEMP_MAX_VALID) {
        // Re-read once if value seems out of range
        delay(TEMP_REREAD_DELAY_MS);
        sensorValue = analogRead(_sensorPin);
        voltage = (float)sensorValue / ESP32_ADC_RESOLUTION * ESP32_ADC_VREF;
        voltageMillivolts = voltage * 1000.0f;
        temperatureC = (voltageMillivolts - TMP36_OFFSET_MV) / TMP36_MV_PER_CELSIUS;
    }
    
    return temperatureC;
}