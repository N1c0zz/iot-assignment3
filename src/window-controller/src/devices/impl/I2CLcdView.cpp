#include "../api/I2CLcdView.h"
#include "config/config.h"

I2CLcdView::I2CLcdView(uint8_t i2cAddress, uint8_t cols, uint8_t rows)
    : lcd(i2cAddress, cols, rows)
    , lastUpdateTimeMs(0)
    , prevIsAutoMode(false)
    , prevWindowPercentage(-1)          // Invalid initial value to force first update
    , prevCurrentTemperature(INVALID_TEMPERATURE)
    , prevIsAlarmState(false)
    , forceUpdate(true)                 // Force complete refresh on first update
{
    // Constructor body intentionally minimal - initialization in setup()
}

void I2CLcdView::setup() {
    // Initialize I2C LCD communication
    lcd.init();
    
    // Enable backlight for visibility
    lcd.backlight();
    
    // Display boot message immediately
    displayBootingMessage();
}

void I2CLcdView::clear() {
    lcd.clear();
    forceUpdate = true;  // Force complete refresh on next update
}

void I2CLcdView::displayBootingMessage() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Booting Sys..."));
}

void I2CLcdView::displayReadyMessage() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("System Ready"));
    
    // Display message briefly, then clear for normal operation
    delay(1000);
    clear();
}

void I2CLcdView::update(bool isAutoMode, int windowPercentage, float currentTemperature, bool isAlarmState) {
    unsigned long currentTimeMs = millis();

    // Determine if display content has changed since last update
    bool modeChanged = (isAutoMode != prevIsAutoMode);
    bool positionChanged = (windowPercentage != prevWindowPercentage);
    bool temperatureChanged = (!isAutoMode && 
                              (abs(currentTemperature - prevCurrentTemperature) > TEMPERATURE_UPDATE_THRESHOLD));

    // Check if update is needed based on changes or refresh interval
    bool updateNeeded = forceUpdate || 
                       modeChanged || 
                       positionChanged || 
                       temperatureChanged ||
                       (currentTimeMs - lastUpdateTimeMs >= LCD_REFRESH_INTERVAL_MS);

    // Check if alarm state changed
    bool alarmStateChanged = (isAlarmState != prevIsAlarmState);

    // Skip update if not needed
    if (!updateNeeded) {
        return;
    }

    // If system is in alarm state, show alarm message
    if (isAlarmState) {
        // Only update if alarm state just changed or force update
        if (alarmStateChanged || forceUpdate) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("ALARM STATE"));
            lcd.setCursor(0, 1);
            lcd.print(F("Reset Required"));
            forceUpdate = false;
        }
        prevIsAlarmState = isAlarmState;
        return;
    }

    // If just exited alarm state, clear display
    if (alarmStateChanged && !isAlarmState) {
        lcd.clear();
        forceUpdate = true;
    }

    // Clear display if mode changed or force update requested
    if (forceUpdate || modeChanged) {
        lcd.clear();
    }
    forceUpdate = false;

    // Update previous alarm state at the end
    prevIsAlarmState = isAlarmState;

    //=========================================================================
    // LINE 0: OPERATIONAL MODE DISPLAY
    //=========================================================================
    
    lcd.setCursor(0, 0);
    lcd.print(F("Mode: "));
    
    String modeString = isAutoMode ? F("AUTO  ") : F("MANUAL");
    lcd.print(modeString);
    
    // Clear remaining characters on line if needed
    clearRestOfLine(0, 6 + modeString.length());

    //=========================================================================
    // LINE 1: WINDOW POSITION DISPLAY
    //=========================================================================
    
    lcd.setCursor(0, 1);
    lcd.print(F("Pos: "));
    
    String positionString = String(windowPercentage) + "%";
    lcd.print(positionString);
    
    // Clear remaining characters on line if needed
    clearRestOfLine(1, 5 + positionString.length());

    //=========================================================================
    // LINE 2: TEMPERATURE DISPLAY (MANUAL MODE ONLY)
    //=========================================================================
    
    if (LCD_ROWS >= 3) {  // Ensure display has at least 3 rows
        lcd.setCursor(0, 2);
        
        if (!isAutoMode) {  // Show temperature only in MANUAL mode
            if (currentTemperature > INVALID_TEMPERATURE + 100.0f) {  // Valid temperature
                lcd.print(F("Temp: "));
                lcd.print(currentTemperature, 1);  // One decimal place
                lcd.print(F(" C"));
                
                // Calculate and clear remaining characters
                int tempDisplayLength = 6 + String(currentTemperature, 1).length() + 2;
                clearRestOfLine(2, tempDisplayLength);
            } else {
                // Invalid temperature - show placeholder
                lcd.print(F("Temp: --- C"));
                clearRestOfLine(2, 11);
            }
        } else {
            // AUTOMATIC mode - clear temperature line
            clearRestOfLine(2, 0);
        }
    }
    
    // Store current values for next comparison
    lastUpdateTimeMs = currentTimeMs;
    prevIsAutoMode = isAutoMode;
    prevWindowPercentage = windowPercentage;
    prevCurrentTemperature = currentTemperature;
}

void I2CLcdView::clearRestOfLine(int row, int startColumn) {
    // Calculate number of characters to clear
    int charactersToClear = LCD_COLUMNS - startColumn;
    
    // Clear remaining characters with spaces
    for (int i = 0; i < charactersToClear; i++) {
        lcd.print(F(" "));
    }
}