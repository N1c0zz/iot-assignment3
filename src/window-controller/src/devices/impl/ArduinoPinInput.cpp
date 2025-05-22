#include "../api/ArduinoPinInput.h"
#include "config/config.h" // Per POT_READ_CHANGE_THRESHOLD se usato qui

ArduinoPinInput::ArduinoPinInput(int buttonPin, int potPin, unsigned long debounceDelay)
    : modeButtonPin(buttonPin),
      potentiometerPin(potPin),
      buttonDebounceDelayMs(debounceDelay),
      lastButtonStateReading(HIGH), // Assumendo INPUT_PULLUP
      lastDebounceEventTimeMs(0)
      // lastPotRawValue(0)
      {}

void ArduinoPinInput::setup() {
    pinMode(modeButtonPin, INPUT_PULLUP);
    pinMode(potentiometerPin, INPUT);
    // lastPotRawValue = analogRead(potentiometerPin); // Inizializza se necessario
}

bool ArduinoPinInput::isModeButtonPressed() {
    bool eventDetected = false;
    int currentButtonReading = digitalRead(modeButtonPin);

    if (currentButtonReading != lastButtonStateReading) {
        lastDebounceEventTimeMs = millis(); // Resetta il timer di debounce
    }

    if ((millis() - lastDebounceEventTimeMs) > buttonDebounceDelayMs) {
        // Se lo stato del bottone è cambiato stabilmente ed è DIVERSO dall'ultimo stato che ha generato un evento
        // e se il bottone è premuto (LOW perché INPUT_PULLUP)
        if (currentButtonReading == LOW && lastButtonStateReading == HIGH) { // Transizione da non premuto a premuto
            eventDetected = true;
        }
    }
    lastButtonStateReading = currentButtonReading; // Salva la lettura attuale per il prossimo ciclo
    return eventDetected;
}

int ArduinoPinInput::getPotentiometerPercentage() {
    int potRawValue = analogRead(potentiometerPin);
    return map(potRawValue, 0, 1023, 0, 100);
}

/*
// Esempio se si volesse implementare "if_changed" qui:
int ArduinoPinInput::getPotentiometerPercentageIfChanged(int& lastKnownPercentageOutput) {
    int potRawValue = analogRead(potentiometerPin);
    if (abs(potRawValue - lastPotRawValue) > POT_READ_CHANGE_THRESHOLD) { // Usa config.h
        lastPotRawValue = potRawValue;
        lastKnownPercentageOutput = map(potRawValue, 0, 1023, 0, 100);
        return lastKnownPercentageOutput; // o solo un flag true/false e aggiorna il riferimento
    }
    return -1; // o un flag false
}
*/