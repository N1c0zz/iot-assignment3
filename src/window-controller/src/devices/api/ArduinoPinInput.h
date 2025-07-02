#ifndef ARDUINO_PIN_INPUT_H
#define ARDUINO_PIN_INPUT_H

#include "UserInputSource.h"
#include "config/config.h"
#include <Arduino.h>

/**
 * @class ArduinoPinInput
 * @brief Arduino GPIO implementation of UserInputSource interface
 * 
 * This class provides concrete user input handling for Arduino platforms,
 * implementing button debouncing and potentiometer filtering. It uses
 * Arduino GPIO pins with appropriate pull-up configurations.
 */
class ArduinoPinInput : public UserInputSource {
public:
    /**
     * @brief Construct Arduino input handler
     * 
     * @param buttonPin Digital pin number for mode button (will use INPUT_PULLUP)
     * @param potPin Analog pin number for potentiometer (A0, A1, etc.)
     * @param debounceDelay Button debounce delay in milliseconds
     */
    ArduinoPinInput(int buttonPin, int potPin, unsigned long debounceDelay);

    /**
     * @brief Default destructor
     */
    virtual ~ArduinoPinInput() = default;

    // UserInputSource interface implementation
    void setup() override;
    bool isModeButtonPressed() override;
    int getPotentiometerPercentage() override;

private:
    int modeButtonPin;                        ///< Digital pin for mode button
    int potentiometerPin;                     ///< Analog pin for potentiometer
    unsigned long buttonDebounceDelayMs;      ///< Debounce delay in milliseconds
    int lastButtonStateReading;               ///< Last raw button pin reading
    int debouncedButtonState;                 ///< Stable debounced button state
    unsigned long lastDebounceEventTimeMs;    ///< Timestamp of last state change
    int potReadings[POT_NUM_SAMPLES];         ///< Circular buffer for readings
    int potReadIndex;                         ///< Current buffer index
    long potTotal;                            ///< Sum of current buffer contents
};

#endif // ARDUINO_PIN_INPUT_H