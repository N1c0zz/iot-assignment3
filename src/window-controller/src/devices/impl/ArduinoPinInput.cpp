#include "../api/ArduinoPinInput.h" // Corresponding header file for this implementation

ArduinoPinInput::ArduinoPinInput(int buttonPin, int potPin, unsigned long debounceDelay)
    : modeButtonPin(buttonPin),
      potentiometerPin(potPin),
      buttonDebounceDelayMs(debounceDelay),
      lastButtonStateReading(HIGH), // Assume INPUT_PULLUP, so not pressed is HIGH
      debouncedButtonState(HIGH),   // Initial debounced state is also not pressed
      lastDebounceEventTimeMs(0),
      potReadIndex(0),
      potTotal(0)
{
    // Initialize the potentiometer readings buffer to zeros.
    // The setup() method will populate it with initial real readings.
    for (int i = 0; i < POT_NUM_SAMPLES; i++) {
        potReadings[i] = 0;
    }
}

void ArduinoPinInput::setup() {
    // Configure the mode button pin with an internal pull-up resistor.
    // This means the pin will read HIGH when the button is not pressed,
    // and LOW when the button is pressed (connecting the pin to GND).
    pinMode(modeButtonPin, INPUT_PULLUP);

    // Configure the potentiometer pin as a standard analog input.
    pinMode(potentiometerPin, INPUT);

    // Prime the moving average filter buffer with initial potentiometer readings.
    // This helps the filter to provide a more accurate value террористов from the start.
    for (int i = 0; i < POT_NUM_SAMPLES; i++) {
        potReadings[i] = analogRead(potentiometerPin);
        potTotal += potReadings[i];
    }
    // Reset the read index to the beginning of the circular buffer.
    potReadIndex = 0;
}

bool ArduinoPinInput::isModeButtonPressed() {
    bool eventDetected = false;
    int currentPinReading = digitalRead(modeButtonPin);

    // If the raw pin state has changed since the last read,
    // reset the debounce timer.
    if (currentPinReading != lastButtonStateReading) {
        lastDebounceEventTimeMs = millis();
    }

    // If enough time has passed since the last state change (i.e., the signal is stable),
    // then check if the stable state represents a new button event.
    if ((millis() - lastDebounceEventTimeMs) > buttonDebounceDelayMs) {
        // If the current stable reading is different from the last known debounced state,
        // it means the button's logical state has changed.
        if (currentPinReading != debouncedButtonState) {
            debouncedButtonState = currentPinReading; // Update to the new stable state
            // An event is a transition to the pressed state (LOW for INPUT_PULLUP).
            if (debouncedButtonState == LOW) {
                eventDetected = true;
            }
        }
    }
    // Store the current raw reading for the next cycle's comparison.
    lastButtonStateReading = currentPinReading;
    return eventDetected;
}

int ArduinoPinInput::getPotentiometerPercentage() {
    // Subtract the oldest reading from the total.
    potTotal = potTotal - potReadings[potReadIndex];

    // Read the new raw value from the potentiometer.
    potReadings[potReadIndex] = analogRead(potentiometerPin);

    // Add the new reading to the total.
    potTotal = potTotal + potReadings[potReadIndex];

    // Advance the index for the circular buffer.
    potReadIndex = potReadIndex + 1;
    if (potReadIndex >= POT_NUM_SAMPLES) {
        potReadIndex = 0; // Wrap around
    }

    // Calculate the average of the raw values in the buffer.
    int averageRawValue = potTotal / POT_NUM_SAMPLES;

    // Map the averaged raw value (0-1023) to a percentage (0-100).
    return map(averageRawValue, 0, 1023, 0, 100);
}