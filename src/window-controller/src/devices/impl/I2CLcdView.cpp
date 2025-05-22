#include "../api/I2CLcdView.h"
#include "config/config.h" // Per le costanti globali LCD

I2CLcdView::I2CLcdView(uint8_t i2cAddress, uint8_t cols, uint8_t rows)
    : lcd(i2cAddress, cols, rows),
      lastUpdateTimeMs(0),
      prevIsAutoMode(false), // Valori iniziali che forzeranno il primo update
      prevWindowPercentage(-1),
      prevCurrentTemperature(-1000.0f),
      forceUpdate(true) {}

void I2CLcdView::setup() {
    lcd.init();
    lcd.backlight();
    displayBootingMessage(); // Mostra messaggio di avvio
}

void I2CLcdView::clear() {
    lcd.clear();
    forceUpdate = true; // Forza il prossimo aggiornamento completo
}

void I2CLcdView::displayBootingMessage() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Booting Sys...")); // Usa F() per stringhe costanti
}

void I2CLcdView::displayReadyMessage() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("System Ready"));
    delay(1000); // Breve pausa per visualizzare
    this->clear(); // Pulisci per la visualizzazione normale e forza il prossimo update
}

void I2CLcdView::update(bool isAutoMode, int windowPercentage, float currentTemperature) {
    unsigned long currentTimeMs = millis();

    // Controlla se è necessario un aggiornamento
    bool stateChanged = (isAutoMode != prevIsAutoMode) ||
                        (windowPercentage != prevWindowPercentage) ||
                        (!isAutoMode && (abs(currentTemperature - prevCurrentTemperature) > 0.05f)); // Piccola tolleranza per float

    if (!forceUpdate && !stateChanged && (currentTimeMs - lastUpdateTimeMs < LCD_REFRESH_INTERVAL_MS)) {
        return; // Nessun aggiornamento necessario
    }

    // Se è passato troppo tempo o lo stato è cambiato o è forzato, aggiorna.
    // Non è necessario pulire sempre se gestiamo bene le sovrascritture.
    // Per semplicità, puliamo se la modalità è cambiata.
    if (forceUpdate || (isAutoMode != prevIsAutoMode)) {
        lcd.clear();
    }
    forceUpdate = false; // Resetta il flag di forzatura

    // Riga 1: Modalità
    lcd.setCursor(0, 0);
    lcd.print(F("Mode: "));
    lcd.print(isAutoMode ? F("AUTO  ") : F("MANUAL")); // Spazi per pulire eventuale testo più lungo

    // Riga 2: Posizione e Temperatura (se manuale)
    lcd.setCursor(0, 1);
    lcd.print(F("Pos: "));
    String posStr = String(windowPercentage) + "%";
    lcd.print(posStr);
    // Pulisci il resto della riga per la posizione
    for (int i = posStr.length(); i < 5; i++) { // Fino a "100% "
        lcd.print(" ");
    }

    if (!isAutoMode) {
        lcd.setCursor(8, 1); // Adatta la posizione per la temperatura
        if (currentTemperature > -990.0f) { // Valore valido
            lcd.print(F("T:"));
            lcd.print(currentTemperature, 1); // 1 cifra decimale
            lcd.print(F("C")); // Aggiungi unità
        } else {
            lcd.print(F("T:--- C"));
        }
    } else {
        // Pulisci l'area della temperatura se si era in manuale
        lcd.setCursor(8, 1);
        lcd.print(F("        ")); // 8 spazi per coprire "T:XX.XC"
    }

    lastUpdateTimeMs = currentTimeMs;
    prevIsAutoMode = isAutoMode;
    prevWindowPercentage = windowPercentage;
    prevCurrentTemperature = currentTemperature;
}