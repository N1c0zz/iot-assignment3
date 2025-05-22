#ifndef ARDUINO_PIN_INPUT_H
#define ARDUINO_PIN_INPUT_H

#include "UserInputSource.h"
#include <Arduino.h>

class ArduinoPinInput : public UserInputSource {
public:
    ArduinoPinInput(int buttonPin, int potPin, unsigned long debounceDelay);
    void setup() override;
    bool isModeButtonPressed() override;
    int getPotentiometerPercentage() override;
    // int getPotentiometerPercentageIfChanged(int& lastReadPercentage) override; // Se si implementa

private:
    int modeButtonPin;
    int potentiometerPin;
    unsigned long buttonDebounceDelayMs;

    // Stato per il debounce del bottone
    int lastButtonStateReading;
    unsigned long lastDebounceEventTimeMs;

    // Stato per il potenziometro (se si implementa "if_changed" qui)
    // int lastPotRawValue;
};

#endif // ARDUINO_PIN_INPUT_H