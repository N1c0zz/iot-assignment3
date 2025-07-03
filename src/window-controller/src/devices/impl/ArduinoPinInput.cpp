#include "../api/ArduinoPinInput.h"
#include "config/config.h"

ArduinoPinInput::ArduinoPinInput(int buttonPin, int potPin, unsigned long debounceDelay)
    : modeButtonPin(buttonPin)
    , potentiometerPin(potPin)
    , buttonDebounceDelayMs(debounceDelay)
    , lastButtonStateReading(HIGH)      // INPUT_PULLUP default state
    , debouncedButtonState(HIGH)        // Initially not pressed
    , lastDebounceEventTimeMs(0)
    , potReadIndex(0)
    , potTotal(0)
{
    // Initialize potentiometer readings buffer to zero
    // Will be populated with real readings during setup()
    for (int i = 0; i < POT_NUM_SAMPLES; i++) {
        potReadings[i] = 0;
    }
}

void ArduinoPinInput::setup() {
    // Configure button pin with internal pull-up resistor
    // This means: HIGH when not pressed, LOW when pressed (connected to GND)
    pinMode(modeButtonPin, INPUT_PULLUP);

    // Configure potentiometer pin as analog input
    pinMode(potentiometerPin, INPUT);

    // Prime the moving average filter with initial readings
    // This provides immediate stable output rather than starting from zero
    for (int i = 0; i < POT_NUM_SAMPLES; i++) {
        potReadings[i] = analogRead(potentiometerPin);
        potTotal += potReadings[i];
    }
    
    // Reset buffer index to beginning
    potReadIndex = 0;
}

bool ArduinoPinInput::isModeButtonPressed() {
    bool pressEventDetected = false;
    int currentPinReading = digitalRead(modeButtonPin);

    // Reset debounce timer if pin state has changed
    if (currentPinReading != lastButtonStateReading) {
        lastDebounceEventTimeMs = millis();
    }

    // Check if enough time has passed for signal to be stable
    if ((millis() - lastDebounceEventTimeMs) > buttonDebounceDelayMs) {
        
        // If stable reading differs from last known debounced state,
        // then the button's logical state has actually changed
        if (currentPinReading != debouncedButtonState) {
            debouncedButtonState = currentPinReading;
            
            // Detect press event: transition to LOW (pressed) state
            // Due to INPUT_PULLUP: HIGH = released, LOW = pressed
            if (debouncedButtonState == LOW) {
                pressEventDetected = true;
            }
        }
    }
    
    // Store current reading for next cycle comparison
    lastButtonStateReading = currentPinReading;
    
    return pressEventDetected;
}

int ArduinoPinInput::getPotentiometerPercentage() {
    // Remove oldest reading from running total
    potTotal -= potReadings[potReadIndex];

    // Read new value and store in circular buffer
    potReadings[potReadIndex] = analogRead(potentiometerPin);

    // Add new reading to running total
    potTotal += potReadings[potReadIndex];

    // Advance circular buffer index
    potReadIndex++;
    if (potReadIndex >= POT_NUM_SAMPLES) {
        potReadIndex = 0;  // Wrap around to beginning
    }

    // Calculate moving average
    int averageRawValue = potTotal / POT_NUM_SAMPLES;

    // Map from practical ADC range to percentage range (0-100)
    // Use a slightly reduced range to account for physical limitations
    int mappedValue = map(averageRawValue, 10, 1013, 0, 100);

    // Constrain to ensure to never go outside 0-100 range
    return constrain(mappedValue, 0, 100);
}