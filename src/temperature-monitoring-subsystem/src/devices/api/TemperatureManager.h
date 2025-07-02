#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

/**
 * @class TemperatureManager
 * @brief Interface for reading temperature data from a sensor.
 * 
 * Provides a contract for initializing a temperature sensor and reading
 * temperature values. Abstracts the underlying sensor hardware implementation.
 */
class TemperatureManager {
public:
    /**
     * @brief Virtual destructor for proper cleanup.
     */
    virtual ~TemperatureManager() {}

    /**
     * @brief Initializes the temperature sensor hardware.
     * Must be called once during system startup before reading values.
     */
    virtual void setup() = 0;

    /**
     * @brief Reads the current temperature from the sensor.
     * @return The current temperature value in Celsius.
     */      
    virtual float readTemperature() = 0;
};

#endif // TEMPERATURE_MANAGER_H