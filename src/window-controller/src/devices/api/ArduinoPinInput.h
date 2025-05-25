#ifndef ARDUINO_PIN_INPUT_H
#define ARDUINO_PIN_INPUT_H

#include "UserInputSource.h" // The interface this class implements
#include <Arduino.h>         // For pinMode, digitalRead, analogRead, millis

/**
 * @class ArduinoPinInput
 * @brief Implements UserInputSource for reading a button and a potentiometer
 *        connected to Arduino pins.
 *
 * This class handles button debouncing and potentiometer reading, including
 * a simple moving average filter for the potentiometer to reduce noise.
 */
class ArduinoPinInput : public UserInputSource {
public:
    /**
     * @brief Constructor for ArduinoPinInput.
     * @param buttonPin The Arduino digital pin number for the mode button.
     * @param potPin The Arduino analog pin number for the potentiometer.
     * @param debounceDelay The debounce delay in milliseconds for the button.
     */
    ArduinoPinInput(int buttonPin, int potPin, unsigned long debounceDelay);

    /**
     * @brief Sets up the button and potentiometer pins.
     *        Initializes pin modes and necessary internal states.
     */
    void setup() override;

    /**
     * @brief Detects a debounced press of the mode button.
     * @return True if a debounced button press has occurred, false otherwise.
     */
    bool isModeButtonPressed() override;

    /**
     * @brief Reads the potentiometer, applies a moving average filter, and returns the value as a percentage.
     * @return The filtered potentiometer value as a percentage (0-100).
     */
    int getPotentiometerPercentage() override;

private:
    // Pin configuration
    int modeButtonPin;                      ///< Arduino pin for the mode selection button.
    int potentiometerPin;                   ///< Arduino pin for the potentiometer.
    unsigned long buttonDebounceDelayMs;    ///< Debounce time for the button in milliseconds.

    // State for button debouncing
    int lastButtonStateReading;             ///< Last raw reading of the button pin.
    int debouncedButtonState;               ///< The stable state of the button after debouncing.
    unsigned long lastDebounceEventTimeMs;  ///< Timestamp of the last button state change.

    // Members for potentiometer moving average filter
    static const int POT_NUM_SAMPLES = 5;   ///< Number of samples for the potentiometer's moving average.
    int potReadings[POT_NUM_SAMPLES];       ///< Array to store recent potentiometer readings.
    int potReadIndex;                       ///< Current index in the potReadings array.
    long potTotal;                          ///< Sum of the current readings in potReadings.
};

#endif // ARDUINO_PIN_INPUT_H