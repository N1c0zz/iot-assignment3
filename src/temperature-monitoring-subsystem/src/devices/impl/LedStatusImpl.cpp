#include "../api/LedStatusImpl.h"
#include <Arduino.h> // Per pinMode, digitalWrite, delay

LedStatusImpl::LedStatusImpl(int greenLedPin, int redLedPin)
    : _greenLedPin(greenLedPin), _redLedPin(redLedPin) {
    // Il costruttore inizializza i pin membri.
    // La configurazione effettiva dei pin hardware avviene in setup().
}

void LedStatusImpl::setup() {
    pinMode(_greenLedPin, OUTPUT);
    pinMode(_redLedPin, OUTPUT);
    turnLedsOff(); // Stato iniziale definito: LED spenti.
    Serial.println("LEDs: OOP Setup completato (Esterni Standard).");
}

void LedStatusImpl::turnLedsOff() {
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, LOW);
}

void LedStatusImpl::indicateSystemBoot() {
    // Fornisce un feedback visivo immediato all'avvio.
    digitalWrite(_greenLedPin, HIGH);
    digitalWrite(_redLedPin, HIGH);
    delay(250);
    turnLedsOff();
    Serial.println("LEDs: Boot indication complete.");
}

void LedStatusImpl::indicateOperational() {
    // Verde acceso, Rosso spento: indica che il sistema funziona correttamente.
    digitalWrite(_greenLedPin, HIGH);
    digitalWrite(_redLedPin, LOW);
}

void LedStatusImpl::indicateNetworkError() {
    // Rosso acceso, Verde spento: indica problemi di connettività o errori critici.
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, HIGH);
}

void LedStatusImpl::indicateWifiConnecting() {
    // Indica il tentativo di connessione WiFi.
    // Attualmente usa lo stesso pattern di NetworkError.
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, HIGH);
}

void LedStatusImpl::indicateMqttConnecting() {
    // Indica il tentativo di connessione MQTT.
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, HIGH);
}