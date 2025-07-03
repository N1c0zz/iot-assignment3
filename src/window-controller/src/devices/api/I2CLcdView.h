#ifndef I2C_LCD_VIEW_H
#define I2C_LCD_VIEW_H

#include "LcdView.h"
#include <LiquidCrystal_I2C.h>
#include "config/config.h"

/**
 * @class I2CLcdView
 * @brief I2C LCD implementation of LcdView interface
 * 
 * This class provides concrete LCD display control using I2C communication.
 * It implements optimized display updates that minimize flickering by
 * tracking previous display state and only updating changed content.
 */
class I2CLcdView : public LcdView {
public:
    /**
     * @brief Construct I2C LCD controller
     * 
     * @param i2cAddress I2C address of LCD module
     * @param cols Number of character columns on LCD
     * @param rows Number of character rows on LCD
     */
    I2CLcdView(uint8_t i2cAddress, uint8_t cols, uint8_t rows);

    /**
     * @brief Default destructor
     */
    virtual ~I2CLcdView() = default;

    // LcdView interface implementation
    void setup() override;
    void clear() override;
    void displayBootingMessage() override;
    void displayReadyMessage() override;
    void update(bool isAutoMode, int windowPercentage, float currentTemperature, bool isAlarmState = false) override;

private:
    LiquidCrystal_I2C lcd;              ///< I2C LCD library instance
    unsigned long lastUpdateTimeMs;     ///< Timestamp of last display update
    
    // Previous display values for change detection
    bool prevIsAutoMode;                ///< Previously displayed mode
    int prevWindowPercentage;           ///< Previously displayed window position
    float prevCurrentTemperature;       ///< Previously displayed temperature
    bool prevIsAlarmState;              ///< Previously displayed alarm state
    
    bool forceUpdate;                   ///< Flag to force complete display refresh

    /** @brief Sentinel value for invalid temperature */
    static constexpr float INVALID_TEMPERATURE = -1000.0f;
    
    /** @brief Temperature change threshold for display update */
    static constexpr float TEMPERATURE_UPDATE_THRESHOLD = 0.05f;

    /**
     * @brief Clear remaining characters on LCD line
     * 
     * Clears characters from specified column to end of line to prevent
     * display artifacts when new text is shorter than previous text.
     * 
     * @param row LCD row number (0-based)
     * @param startColumn Starting column for clearing (0-based)
     */
    void clearRestOfLine(int row, int startColumn);
};

#endif // I2C_LCD_VIEW_H