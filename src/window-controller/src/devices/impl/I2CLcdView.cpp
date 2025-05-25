#include "../api/I2CLcdView.h" // Corresponding header
#include "config/config.h"    // For LCD_COLUMNS, LCD_ROWS, LCD_REFRESH_INTERVAL_MS

I2CLcdView::I2CLcdView(uint8_t i2cAddress, uint8_t cols, uint8_t rows)
    : lcd(i2cAddress, cols, rows), // Initialize the LiquidCrystal_I2C object
      lastUpdateTimeMs(0),
      prevIsAutoMode(false),       // Initial values designed to trigger the first update
      prevWindowPercentage(-1),
      prevCurrentTemperature(-1000.0f), // Sentinel for invalid temperature
      forceUpdate(true) {}

void I2CLcdView::setup() {
    lcd.init();      // Initialize the LCD library
    lcd.backlight(); // Turn on the LCD backlight
    displayBootingMessage();
}

void I2CLcdView::clear() {
    lcd.clear();
    forceUpdate = true; // Force a full redraw on the next update call
}

void I2CLcdView::displayBootingMessage() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Booting Sys...")); // Use F() macro for constant strings to save SRAM
}

void I2CLcdView::displayReadyMessage() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("System Ready"));
    delay(1000); // Display for a short period
    this->clear(); // Clear the screen afterwards, preparing for normal operation
}

void I2CLcdView::update(bool isAutoMode, int windowPercentage, float currentTemperature) {
    unsigned long currentTimeMs = millis();

    // Determine if the displayed state has changed since the last update
    bool stateChanged = (isAutoMode != prevIsAutoMode) ||
                        (windowPercentage != prevWindowPercentage) ||
                        (!isAutoMode && (abs(currentTemperature - prevCurrentTemperature) > 0.05f)); // Small tolerance for float comparison

    // Only update if forced, state changed, or refresh interval passed
    if (!forceUpdate && !stateChanged && (currentTimeMs - lastUpdateTimeMs < LCD_REFRESH_INTERVAL_MS)) {
        return; // No update needed
    }

    // Clear the entire screen if the mode changes or if a force update is requested.
    // This simplifies redraw logic but can cause more flickering than selective clearing.
    if (forceUpdate || (isAutoMode != prevIsAutoMode)) {
        lcd.clear();
    }
    forceUpdate = false; // Reset force flag

    // Line 0: Display current operational mode
    lcd.setCursor(0, 0);
    lcd.print(F("Mode: "));
    String modeStr = isAutoMode ? F("AUTO  ") : F("MANUAL"); // Padding spaces for clean overwrite
    lcd.print(modeStr);
    // Clear remaining part of the line if previous string was longer
    for (int i = (6 + modeStr.length()); i < LCD_COLUMNS; i++) { // "Mode: " is 6 chars
        lcd.print(F(" "));
    }

    // Line 1: Display window position
    lcd.setCursor(0, 1);
    lcd.print(F("Pos: "));
    String posStr = String(windowPercentage) + "%";
    lcd.print(posStr);
    // Clear remaining part of the line
    for (int i = (5 + posStr.length()); i < LCD_COLUMNS; i++) { // "Pos: " is 5 chars
        lcd.print(F(" "));
    }

    // Line 2: Display temperature (only in MANUAL mode)
    if (LCD_ROWS >= 3) { // Check if the display has at least 3 rows
        lcd.setCursor(0, 2);
        if (!isAutoMode) {
            if (currentTemperature > -990.0f) { // Check for a valid temperature reading
                lcd.print(F("Temp: "));
                lcd.print(currentTemperature, 1); // One decimal place
                lcd.print(F(" C"));
                // Clear remaining part of the line
                // Calculate current length: "Temp: " (6) + number + " C" (2)
                int currentDisplayLength = 6 + String(currentTemperature, 1).length() + 2;
                for (int i = currentDisplayLength; i < LCD_COLUMNS; i++) {
                    lcd.print(F(" "));
                }
            } else {
                lcd.print(F("Temp: --- C")); // Placeholder for invalid temperature
                // Clear remaining part of the line
                for (int i = strlen("Temp: --- C"); i < LCD_COLUMNS; i++) {
                    lcd.print(F(" "));
                }
            }
        } else {
            // In AUTOMATIC mode, clear the temperature line
            for (int i = 0; i < LCD_COLUMNS; i++) {
                lcd.print(F(" "));
            }
        }
    }

    // Store current state for next comparison
    lastUpdateTimeMs = currentTimeMs;
    prevIsAutoMode = isAutoMode;
    prevWindowPercentage = windowPercentage;
    prevCurrentTemperature = currentTemperature;
}