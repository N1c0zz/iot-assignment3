#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

/**
 * @class TemperatureManager
 * @brief Interface for reading temperature data.
 * Provides a contract for initializing a temperature sensor and reading its values.
 */
class TemperatureManager {
public:
    /**
    * @brief Virtual destructor for proper cleanup.
    */
    virtual ~TemperatureManager() {}

    /**
    * @brief Initializes the temperature sensor hardware.
    * Must be called once during system startup.
    */
    virtual void setup() = 0;
    /**
    * @brief Reads the current temperature from the sensor.
    * @return The current temperature value, typically in Celsius.   
    */      
    virtual float readTemperature() = 0;
};

#endif // TEMPERATURE_MANAGER_H