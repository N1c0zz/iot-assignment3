#include "../api/TemperatureManagerImpl.h"
#include <Arduino.h> // Per pinMode, analogRead

TemperatureManagerImpl::TemperatureManagerImpl(int sensorPin) : _sensorPin(sensorPin) {
    // Il costruttore inizializza il pin del sensore.
    // La configurazione del pin hardware avviene in setup().
}

void TemperatureManagerImpl::setup() {
    pinMode(_sensorPin, INPUT);
    // Nota: l'ADC dell'ESP32 può richiedere una configurazione di attenuazione
    // per leggere correttamente l'intero range di tensione del TMP36 (0-3.3V).
    // Esempio: analogSetPinAttenuation(_sensorPin, ADC_11db);
    // Per ora, si utilizza la configurazione ADC di default.
    Serial.println("Sensor: OOP Setup completato.");
}

float TemperatureManagerImpl::readTemperature() {
    // Legge il valore grezzo dall'ADC.
    int sensorValue = analogRead(_sensorPin);
    // Converte il valore ADC in tensione.
    // Si assume un ADC a 12 bit (0-4095) e una tensione di riferimento di 3.3V.
    // La precisione può variare; per misure critiche, considerare la calibrazione dell'ADC.
    float voltage = (float)sensorValue / 4095.0 * 3.3;
    // Converte la tensione in gradi Celsius secondo le specifiche del TMP36.
    // Formula: Temp (°C) = (Vout in mV - 500) / 10
    // Che è equivalente a: Temp (°C) = (Vout in V * 100) - 50
    float temperatureC = (voltage * 100.0) - 50.0;
    return temperatureC;
}