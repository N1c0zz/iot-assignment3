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
    int debouncedButtonState;
    unsigned long lastDebounceEventTimeMs;

    // Membri per la media mobile del potenziometro
    static const int POT_NUM_SAMPLES = 5; //numero di valori per la media mobile
    int potReadings[POT_NUM_SAMPLES];
    int potReadIndex;
    long potTotal;
};

#endif // ARDUINO_PIN_INPUT_H