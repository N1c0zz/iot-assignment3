#include "sensor_manager.h"
#include "config.h"
#include <Arduino.h>

void setupSensor() {
  pinMode(TEMP_SENSOR_PIN, INPUT);
  // Considera di impostare l'attenuazione dell'ADC se necessario per il range completo 0-3.3V
  // Ad esempio: analogSetPinAttenuation(TEMP_SENSOR_PIN, ADC_11db);
  // Per ora, usiamo il default.
}

float readTemperature() {
  int sensorValue = analogRead(TEMP_SENSOR_PIN);
  
  // Converti il valore analogico (0-4095 per ADC a 12bit dell'ESP32) in tensione.
  // Questa conversione assume che l'ADC sia configurato per leggere fino a 3.3V (VDD_A)
  // e che la tensione di riferimento effettiva sia 3.3V.
  // Per una maggiore precisione, sarebbe necessaria una calibrazione dell'ADC.
  float voltage = (float)sensorValue / 4095.0 * 3.3; 

  // Converti la tensione in temperatura per TMP36: (Tensione in V * 100) - 50
  float temperatureC = (voltage * 100.0) - 50.0;
  
  return temperatureC;
}