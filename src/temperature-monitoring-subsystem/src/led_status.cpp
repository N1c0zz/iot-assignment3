#include "led_status.h"
#include "config.h" // Per GREEN_LED_PIN, RED_LED_PIN
#include <Arduino.h> // Per pinMode, digitalWrite, delay (usato solo in boot)

// Variabili per eventuale lampeggio non bloccante (non implementato completamente qui)
// bool redLedBlinking = false;
// bool greenLedBlinking = false;
// unsigned long lastBlinkTime = 0;
// int blinkInterval = 500; // ms

void setupLed() {
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  turnLedsOff(); // Inizia con entrambi i LED spenti
  Serial.println("LEDs: Setup completato (Esterni Standard).");
}

void turnLedsOff() {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
}

void indicateSystemBoot() {
  // Breve lampeggio di entrambi per indicare il boot
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, HIGH);
  delay(250);
  turnLedsOff();
  Serial.println("LEDs: Boot indication complete.");
}

// "When the system is working correctly (network ok, sending data ok) the green
// led is on and the red is off"
void indicateOperational() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  // redLedBlinking = false; greenLedBlinking = false; // Se si usasse il lampeggio
  // Serial.println("LEDs: Operational (Green ON, Red OFF)"); // Log opzionale, può essere verboso
}

// "otherwise – in the case of network problems – the red
// led should be on and the green led off."
void indicateNetworkError() {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  // redLedBlinking = false; greenLedBlinking = false;
  // Serial.println("LEDs: Network Error (Red ON, Green OFF)"); // Log opzionale
}

// Comportamento per stati di transizione
// Per ora, gli stati di transizione mostrano lo stesso pattern di errore di rete,
// implicando che il sistema non è ancora pienamente operativo.
void indicateWifiConnecting() {
  // Esempio: Rosso fisso, Verde spento (come un errore di rete finché non connesso)
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  // Serial.println("LEDs: WiFi Connecting (Red ON, Green OFF)");
}

void indicateMqttConnecting() {
  // Esempio: Rosso fisso, Verde spento (come un errore di rete finché non connesso)
  // Oppure potresti fare un pattern diverso, es. Rosso e Verde entrambi accesi (giallo)
  digitalWrite(GREEN_LED_PIN, LOW); // o HIGH per giallo
  digitalWrite(RED_LED_PIN, HIGH);
}

/*
// Esempio di funzione per gestire il lampeggio non bloccante (da chiamare nel loop principale)
void updateLedBlink() {
  if (redLedBlinking || greenLedBlinking) {
    if (millis() - lastBlinkTime > blinkInterval) {
      lastBlinkTime = millis();
      if (redLedBlinking) digitalWrite(RED_LED_PIN, !digitalRead(RED_LED_PIN));
      if (greenLedBlinking) digitalWrite(GREEN_LED_PIN, !digitalRead(GREEN_LED_PIN));
    }
  }
}
*/