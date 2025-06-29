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
    float voltage = (float)sensorValue / 4095.0 * 3.3;
    
    // Converte la tensione in gradi Celsius secondo le specifiche del TMP36.
    float temperatureC = (voltage * 100.0) - 50.0;
    
    // FILTRO PER VALORI ANOMALI
    // TMP36 range tipico: -40°C a +125°C, ma per uso domestico: 0°C a 50°C
    if (temperatureC < -10.0 || temperatureC > 60.0) {
        
        // Per ora: rileggi una volta
        delay(10);
        sensorValue = analogRead(_sensorPin);
        voltage = (float)sensorValue / 4095.0 * 3.3;
        temperatureC = (voltage * 100.0) - 50.0;
    }
    
    return temperatureC;
}