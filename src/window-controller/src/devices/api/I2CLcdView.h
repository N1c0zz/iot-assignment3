#ifndef I2C_LCD_VIEW_H
#define I2C_LCD_VIEW_H

#include "LcdView.h" // Interfaccia
#include <LiquidCrystal_I2C.h> // Libreria specifica
#include "config/config.h"     // Per LCD_REFRESH_INTERVAL_MS

class I2CLcdView : public LcdView {
public:
    I2CLcdView(uint8_t i2cAddress, uint8_t cols, uint8_t rows);
    void setup() override;
    void clear() override;
    void displayBootingMessage() override;
    void displayReadyMessage() override;
    void update(bool isAutoMode, int windowPercentage, float currentTemperature) override;

private:
    LiquidCrystal_I2C lcd;
    unsigned long lastUpdateTimeMs;

    // Variabili per tracciare lo stato precedente e aggiornare solo se necessario
    bool prevIsAutoMode;
    int prevWindowPercentage;
    float prevCurrentTemperature;
    bool forceUpdate; // Per forzare l'aggiornamento al primo ciclo o dopo un clear
};

#endif // I2C_LCD_VIEW_H