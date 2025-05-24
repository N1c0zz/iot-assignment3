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

    bool stateChanged = (isAutoMode != prevIsAutoMode) ||
                        (windowPercentage != prevWindowPercentage) ||
                        (!isAutoMode && (abs(currentTemperature - prevCurrentTemperature) > 0.05f));

    if (!forceUpdate && !stateChanged && (currentTimeMs - lastUpdateTimeMs < LCD_REFRESH_INTERVAL_MS)) {
        return;
    }

    // Pulisci solo se strettamente necessario o se la modalità cambia per evitare flickering
    if (forceUpdate || (isAutoMode != prevIsAutoMode)) {
        lcd.clear(); // Cancella tutto se la modalità cambia o se è forzato
    }
    forceUpdate = false;

    // --- Riga 0: Modalità --- (Assumiamo che LCD_ROWS sia almeno 2)
    lcd.setCursor(0, 0); // Colonna 0, Riga 0
    lcd.print(F("Mode: "));
    String modeStr = isAutoMode ? F("AUTO  ") : F("MANUAL");
    lcd.print(modeStr);
    // Pulisci il resto della riga se necessario (se la stringa precedente era più lunga)
    for (int i = modeStr.length(); i < (LCD_COLUMNS - 6); i++) { // "Mode: " occupa 6 caratteri
        lcd.print(F(" "));
    }


    // --- Riga 1: Posizione Finestra ---
    lcd.setCursor(0, 1); // Colonna 0, Riga 1
    lcd.print(F("Pos: "));
    String posStr = String(windowPercentage) + "%";
    lcd.print(posStr);
    // Pulisci il resto della riga per la posizione
    for (int i = posStr.length(); i < (LCD_COLUMNS - 5); i++) { // "Pos: " occupa 5 caratteri
        lcd.print(F(" "));
    }

    // --- Riga 2: Temperatura (solo in modalità MANUALE) ---
    // Assumiamo che LCD_ROWS sia almeno 3 per questa riga
    if (LCD_ROWS >= 3) {
        lcd.setCursor(0, 2); // Colonna 0, Riga 2
        if (!isAutoMode) { // Mostra temperatura solo in MANUALE
            if (currentTemperature > -990.0f) {
                lcd.print(F("Temp: "));
                lcd.print(currentTemperature, 1);
                lcd.print(F(" C")); // Aggiungi unità
                // Pulisci eventuale testo residuo se il numero è corto
                // (calcola la lunghezza di "Temp: XX.XC")
                int tempLen = 6 + String(currentTemperature, 1).length() + 2; // "Temp: " + num + " C"
                 for (int i = tempLen -6 ; i < (LCD_COLUMNS-6); i++) { // "Temp: " occupa 6 caratteri
                     lcd.print(F(" "));
                 }
            } else {
                lcd.print(F("Temp: --- C    ")); // Assicura di pulire la riga
            }
        } else {
            // Se in AUTO, pulisci la riga della temperatura
            for (int i = 0; i < LCD_COLUMNS; i++) {
                lcd.print(F(" "));
            }
        }
    }

    // --- Riga 3: Libera per usi futuri o vuota ---
    // Se LCD_ROWS è 4, puoi usare la riga 3 per altro o assicurarti che sia pulita
    if (LCD_ROWS >= 4) {
        lcd.setCursor(0, 3); // Colonna 0, Riga 3
        // Esempio: pulisci la riga 3
        // for (int i = 0; i < LCD_COLUMNS; i++) {
        //     lcd.print(F(" "));
        // }
        // Oppure mostra un messaggio fisso o un separatore:
        // lcd.print(F("----------------")); // Se hai 16 colonne
    }


    lastUpdateTimeMs = currentTimeMs;
    prevIsAutoMode = isAutoMode;
    prevWindowPercentage = windowPercentage;
    prevCurrentTemperature = currentTemperature;
}