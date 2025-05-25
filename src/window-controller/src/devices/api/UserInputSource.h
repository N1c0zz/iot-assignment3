#ifndef USER_INPUT_SOURCE_H
#define USER_INPUT_SOURCE_H

/**
 * @class UserInputSource
 * @brief Interface for components provides user input data.
 *
 * This class defines a contract for how the system will interact with
 * user input hardware, like buttons and potentiometers.
 */
class UserInputSource {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~UserInputSource() {}

    /**
     * @brief Initializes the user input source component.
     *        This should typically configure pins and initial states.
     */
    virtual void setup() = 0;

    /**
     * @brief Checks if the mode button has been pressed.
     * @return True if a debounced button press event has occurred since the last call, false otherwise.
     */
    virtual bool isModeButtonPressed() = 0;

    /**
     * @brief Gets the current potentiometer value, mapped to a percentage.
     * @return The potentiometer value as a percentage (0-100), potentially filtered.
     */
    virtual int getPotentiometerPercentage() = 0;
};

#endif // USER_INPUT_SOURCE_H