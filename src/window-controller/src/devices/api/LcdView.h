#ifndef LCD_VIEW_H
#define LCD_VIEW_H

#include <Arduino.h>

/**
 * @class LcdView
 * @brief Abstract interface for LCD display control
 * 
 * This interface defines the contract for controlling an LCD display
 * used to show system status, operational mode, window position,
 * and temperature information to the user.
 */
class LcdView {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~LcdView() = default;

    /**
     * @brief Initialize LCD hardware and display
     */
    virtual void setup() = 0;

    /**
     * @brief Clear entire LCD display
     */
    virtual void clear() = 0;

    /**
     * @brief Display system boot/initialization message
     */
    virtual void displayBootingMessage() = 0;

    /**
     * @brief Display system ready message
     * 
     * Shows a "ready" message after successful system initialization
     * to indicate the system is operational and ready for use.
     */
    virtual void displayReadyMessage() = 0;

    /**
     * @brief Update LCD with current system status
     * 
     * @param isAutoMode true if system is in AUTOMATIC mode, false for MANUAL
     * @param windowPercentage Current window opening percentage (0-100)
     * @param currentTemperature Current temperature reading in Celsius
     */
    virtual void update(bool isAutoMode, int windowPercentage, float currentTemperature) = 0;
};

#endif // LCD_VIEW_H