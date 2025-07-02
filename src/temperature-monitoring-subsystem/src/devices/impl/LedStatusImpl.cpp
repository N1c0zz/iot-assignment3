#include "../api/LedStatusImpl.h"
#include <Arduino.h>

LedStatusImpl::LedStatusImpl(int greenLedPin, int redLedPin)
    : _greenLedPin(greenLedPin), _redLedPin(redLedPin) {
    // Constructor initializes pin member variables.
    // Actual hardware pin configuration is performed in setup().
}

void LedStatusImpl::setup() {
    pinMode(_greenLedPin, OUTPUT);
    pinMode(_redLedPin, OUTPUT);
    turnLedsOff(); // Set initial state: LEDs off
}

void LedStatusImpl::turnLedsOff() {
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, LOW);
}

void LedStatusImpl::indicateSystemBoot() {
    // Brief flash of both LEDs to indicate system startup
    digitalWrite(_greenLedPin, HIGH);
    digitalWrite(_redLedPin, HIGH);
    delay(250);
    turnLedsOff();
}

void LedStatusImpl::indicateOperational() {
    // Green LED on, Red LED off: system operating normally
    digitalWrite(_greenLedPin, HIGH);
    digitalWrite(_redLedPin, LOW);
}

void LedStatusImpl::indicateNetworkError() {
    // Red LED on, Green LED off: network or critical error
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, HIGH);
}

void LedStatusImpl::indicateWifiConnecting() {
    // Red LED on while attempting WiFi connection
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, HIGH);
}

void LedStatusImpl::indicateMqttConnecting() {
    // Red LED on while attempting MQTT connection
    digitalWrite(_greenLedPin, LOW);
    digitalWrite(_redLedPin, HIGH);
}