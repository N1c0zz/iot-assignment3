#ifndef TEMPERATURE_MANAGER_IMPL_H
#define TEMPERATURE_MANAGER_IMPL_H

#include "../api/TemperatureManager.h"
#include "config/config.h"

/**
 * @class TemperatureManagerImpl
 * @brief Implements TemperatureManager for a TMP36 analog sensor.
 * 
 * Handles GPIO pin setup and analog-to-digital conversion for temperature readings
 * from a TMP36 temperature sensor connected to an ESP32 analog input pin.
 */
class TemperatureManagerImpl : public TemperatureManager {
public:
    /**
     * @brief Constructor for TemperatureManagerImpl.
     * @param sensorPin Analog GPIO pin connected to the TMP36 sensor's Vout. 
     *                  Defaults to TEMP_SENSOR_PIN from config.
     */
    TemperatureManagerImpl(int sensorPin = TEMP_SENSOR_PIN);

    /**
     * @brief Virtual destructor.
     */
    virtual ~TemperatureManagerImpl() {}

    void setup() override;
    float readTemperature() override;

private:
    int _sensorPin;  ///< Analog GPIO pin connected to the temperature sensor.
};

#endif // TEMPERATURE_MANAGER_IMPL_H