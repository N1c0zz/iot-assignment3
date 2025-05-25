#ifndef I2C_LCD_VIEW_H
#define I2C_LCD_VIEW_H

#include "LcdView.h"            // The interface this class implements
#include <LiquidCrystal_I2C.h> // Specific library for I2C LCDs
#include "config/config.h"      // For LCD_REFRESH_INTERVAL_MS, LCD_COLUMNS, LCD_ROWS

/**
 * @class I2CLcdView
 * @brief Implements the LcdView interface for controlling an LCD display
 *        via an I2C interface.
 *
 * This class manages the display of system mode, window position, and temperature
 * on a multi-line LCD, optimizing updates to reduce flickering.
 */
class I2CLcdView : public LcdView {
public:
    /**
     * @brief Constructor for I2CLcdView.
     * @param i2cAddress The I2C address of the LCD module.
     * @param cols The number of columns of the LCD display.
     * @param rows The number of rows of the LCD display.
     */
    I2CLcdView(uint8_t i2cAddress, uint8_t cols, uint8_t rows);

    void setup() override;
    void clear() override;
    void displayBootingMessage() override;
    void displayReadyMessage() override;
    void update(bool isAutoMode, int windowPercentage, float currentTemperature) override;

private:
    LiquidCrystal_I2C lcd;              ///< Instance of the I2C LCD library.
    unsigned long lastUpdateTimeMs;     ///< Timestamp of the last LCD update.

    // State variables to track previous display values for optimized updates.
    bool prevIsAutoMode;                ///< Previously displayed mode.
    int prevWindowPercentage;           ///< Previously displayed window percentage.
    float prevCurrentTemperature;       ///< Previously displayed temperature.
    bool forceUpdate;                   ///< Flag to force a full LCD refresh on the next update cycle.
};

#endif // I2C_LCD_VIEW_H