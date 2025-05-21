#ifndef LED_STATUS_H
#define LED_STATUS_H

void setupLed();

// Funzioni di indicazione come da specifiche e per stati intermedi
void indicateOperational();       // Verde acceso, Rosso spento (Sistema OK)
void indicateNetworkError();      // Rosso acceso, Verde spento (Problemi di rete)

// Stati di transizione (comportamento da definire, esempio fornito)
void indicateSystemBoot();
void indicateWifiConnecting();
void indicateMqttConnecting();

void turnLedsOff();             // Funzione ausiliaria per spegnere entrambi

// Se vuoi implementare lampeggi non bloccanti:
// void updateLedBlink(); // da chiamare nel loop principale

#endif // LED_STATUS_H