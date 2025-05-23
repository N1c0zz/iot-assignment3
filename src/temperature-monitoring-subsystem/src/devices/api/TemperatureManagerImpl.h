#ifndef TEMPERATURE_MANAGER_IMPL_H
#define TEMPERATURE_MANAGER_IMPL_H

#include "../api/TemperatureManager.h"
#include "config/config.h" // Per TEMP_SENSOR_PIN

/**
 * @class TemperatureManagerImpl
 * @brief Implements TemperatureManager for a TMP36 analog sensor.
 * Handles GPIO pin setup and analog-to-digital conversion for temperature readings.
 */
class TemperatureManagerImpl : public TemperatureManager {
public:
    /**
    * @brief Constructor for TemperatureManagerImpl.
    * Initializes with the specified or default analog GPIO pin for the TMP36 sensor.
    * @param sensorPin Analog GPIO pin connected to the TMP36 sensor's Vout. Defaults to TEMP_SENSOR_PIN.
    */
    TemperatureManagerImpl(int sensorPin = TEMP_SENSOR_PIN);
    /**
    * @brief Default virtual destructor.
    */
    virtual ~TemperatureManagerImpl() {}

    void setup() override;
    float readTemperature() override;

private:
  int _sensorPin;  ///< Analog GPIO pin connected to the temperature sensor.
};

#endif // TEMPERATURE_MANAGER_IMPL_H