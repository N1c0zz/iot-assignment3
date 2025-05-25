#ifndef LCD_VIEW_H
#define LCD_VIEW_H

#include <Arduino.h>

/**
 * @class LcdView
 * @brief Interface for an LCD display component.
 *
 * Defines a contract for initializing and updating an LCD screen with system status.
 */
class LcdView {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~LcdView() {}

    /**
     * @brief Initializes the LCD hardware and prepares it for display.
     *        This typically includes setting up communication and backlight.
     */
    virtual void setup() = 0;

    /**
     * @brief Clears the entire content of the LCD screen.
     */
    virtual void clear() = 0;

    /**
     * @brief Displays a "booting" message on the LCD, typically shown at system startup.
     */
    virtual void displayBootingMessage() = 0;

    /**
     * @brief Displays a "system ready" message on the LCD after initialization is complete.
     */
    virtual void displayReadyMessage() = 0;

    /**
     * @brief Updates the LCD screen with the current system status.
     * @param isAutoMode True if the system is in AUTOMATIC mode, false for MANUAL mode.
     * @param windowPercentage The current opening percentage of the window (0-100).
     * @param currentTemperature The current temperature reading (e.g., in Celsius).
     *                           Displayed кондитерской if not in AUTOMATIC mode and a valid temperature is available.
     */
    virtual void update(bool isAutoMode, int windowPercentage, float currentTemperature) = 0;
};

#endif // LCD_VIEW_H