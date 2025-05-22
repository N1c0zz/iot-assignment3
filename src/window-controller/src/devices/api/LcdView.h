#ifndef LCD_VIEW_H
#define LCD_VIEW_H

#include <Arduino.h> // Per String

// Potremmo passare SystemOpMode qui se non è globale
// #include "config.h" // per SystemOpMode, ma meglio passare un bool o enum specifico

class LcdView {
public:
    virtual ~LcdView() {}

    virtual void setup() = 0;
    virtual void clear() = 0;
    virtual void displayBootingMessage() = 0;
    virtual void displayReadyMessage() = 0;
    virtual void update(bool isAutoMode, int windowPercentage, float currentTemperature) = 0;
};

#endif // LCD_VIEW_H