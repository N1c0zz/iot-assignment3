#include "../api/ArduinoPinInput.h"
#include "config/config.h" // Per POT_READ_CHANGE_THRESHOLD se usato qui

ArduinoPinInput::ArduinoPinInput(int buttonPin, int potPin, unsigned long debounceDelay)
    : modeButtonPin(buttonPin),
      potentiometerPin(potPin),
      buttonDebounceDelayMs(debounceDelay),
      lastButtonStateReading(HIGH), // Assumendo INPUT_PULLUP
      debouncedButtonState(HIGH),
      lastDebounceEventTimeMs(0),
      // Inizializzazione per la media mobile del potenziometro
      potReadIndex(0),
      potTotal(0)
{
    for (int i = 0; i < POT_NUM_SAMPLES; i++)
    {
        potReadings[i] = 0;
    }
}

void ArduinoPinInput::setup()
{
    // Molti microcontrollori hanno la capacità di abilitare un resistore di pull-up interno
    // per i loro pin digitali. FUNZIONAMENTO:
    // Bottone NON premuto: il circuito verso GND è aperto. Il microcontrollore collega internamente
    // il buttonPin a 5V attraverso il suo resistore di pull-up interno. Quindi il pine legge HIGH,
    // Bottone PREMUTO: il circuito verso GND si chiude. Il buttonPIN è ora collegato direttamente
    // a GND. Il pin legge LOW.
    pinMode(modeButtonPin, INPUT_PULLUP);
    pinMode(potentiometerPin, INPUT);
    // Inizializza il buffer della media mobile con letture reali
    for (int i = 0; i < POT_NUM_SAMPLES; i++) {
        potReadings[i] = analogRead(potentiometerPin);
        potTotal += potReadings[i];
        // delay(5); // Piccolo delay tra le letture iniziali se necessario
    }
    potReadIndex = 0; // Resetta l'indice dopo il riempimento iniziale
}

bool ArduinoPinInput::isModeButtonPressed()
{
    bool eventDetected = false;
    int reading = digitalRead(modeButtonPin);

    // Se la lettura è cambiata, resetta il timer di debounce
    if (reading != lastButtonStateReading)
    {
        lastDebounceEventTimeMs = millis();
    }

    if ((millis() - lastDebounceEventTimeMs) > buttonDebounceDelayMs)
    {
        // Se la lettura è rimasta stabile per più del tempo di debounce
        // e se lo stato stabile attuale è diverso da quello precedente,
        // allora lo stato del bottone è cambiato.
        if (reading != debouncedButtonState)
        {                                   // debouncedButtonState è lo stato stabile precedente
            debouncedButtonState = reading; // Aggiorna allo nuovo stato stabile
            if (debouncedButtonState == LOW)
            { // Se il nuovo stato stabile è PREMUTO
                eventDetected = true;
            }
        }
    }
    lastButtonStateReading = reading; // Salva la lettura corrente per il prossimo ciclo
    return eventDetected;
}

int ArduinoPinInput::getPotentiometerPercentage()
{
    // Sottrai l'ultima lettura vecchia dal totale
    potTotal = potTotal - potReadings[potReadIndex];

    // Leggi il nuovo valore e memorizzalo nell'array
    potReadings[potReadIndex] = analogRead(potentiometerPin);

    // Aggiungi la nuova lettura al totale
    potTotal = potTotal + potReadings[potReadIndex];

    // Avanza all'indice successivo dell'array (circolare)
    potReadIndex = potReadIndex + 1;
    if (potReadIndex >= POT_NUM_SAMPLES) {
        potReadIndex = 0;
    }

    // Calcola la media dei valori grezzi
    int averageRawValue = potTotal / POT_NUM_SAMPLES;

    // Mappa il valore medio grezzo a una percentuale
    return map(averageRawValue, 0, 1023, 0, 100);
}