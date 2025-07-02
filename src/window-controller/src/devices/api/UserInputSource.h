#ifndef USER_INPUT_SOURCE_H
#define USER_INPUT_SOURCE_H

/**
 * @class UserInputSource
 * @brief Abstract interface for user input hardware
 * 
 * This interface defines the contract for accessing user input devices
 * such as buttons and potentiometers. It provides a hardware
 * way to handle user interactions in the window controller system.
 */
class UserInputSource {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~UserInputSource() = default;

    /**
     * @brief Initialize user input hardware
     */
    virtual void setup() = 0;

    /**
     * @brief Check for mode button press event
     * 
     * Detects a debounced button press event. This method should
     * return true only once per physical button press, implementing
     * proper debouncing to filter out mechanical bouncing.
     * 
     * @return true if a new debounced button press occurred since last call
     * @return false if no new button press detected
     */
    virtual bool isModeButtonPressed() = 0;

    /**
     * @brief Get filtered potentiometer reading as percentage
     * 
     * Reads the potentiometer value, applies appropriate filtering
     * (such as moving average), and returns the result as a percentage.
     * 
     * @return Filtered potentiometer value as percentage (0-100)
     */
    virtual int getPotentiometerPercentage() = 0;
};

#endif // USER_INPUT_SOURCE_H