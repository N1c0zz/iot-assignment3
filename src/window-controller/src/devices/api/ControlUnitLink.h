#ifndef CONTROL_UNIT_LINK_H
#define CONTROL_UNIT_LINK_H

#include <Arduino.h>      // For String type
#include "config/config.h" // For SystemOpMode enum (or a more specific FSM state enum)

/**
 * @class ControlUnitLink
 * @brief Interface for a communication link to the system's Control Unit.
 *
 * Defines a contract for sending notifications and receiving commands,
 * abstracting the underlying communication mechanism.
 */
class ControlUnitLink {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ControlUnitLink() {}

    /**
     * @brief Initializes the communication link.
     * @param baudRate The baud rate for serial communication, if applicable.
     */
    virtual void setup(long baudRate) = 0;

    /**
     * @brief Checks if a complete command is available to be read.
     * @return True if a command is ready, false otherwise.
     */
    virtual bool commandAvailable() = 0;

    /**
     * @brief Reads a complete command from the communication link.
     *        Should be called after commandAvailable() returns true.
     * @return The received command as a String. Returns an empty string if no command is ready.
     */
    virtual String readCommand() = 0;

    /**
     * @brief Sends the current potentiometer value to the Control Unit.
     * @param percentage The potentiometer value as a percentage (0-100).
     */
    virtual void sendPotentiometerValue(int percentage) = 0; // Resa virtuale pura

    /**
     * @brief Sends a notification to the Control Unit that the system mode has changed locally.
     * @param newMode The new operational mode of the system.
     */
    virtual void sendModeChangedNotification(SystemOpMode newMode) = 0;

    /**
     * @brief Sends an acknowledgement to the Control Unit after a mode change command was processed.
     * @param acknowledgedMode The operational mode that was successfully set.
     */
    virtual void sendAckModeChange(SystemOpMode acknowledgedMode) = 0;

    // Example of a potential future method, kept for reference.
    // /**
    //  * @brief Sends the current overall system state to the Control Unit.
    //  * @param mode The current operational mode.
    //  * @param windowPercentage The current window opening percentage.
    //  */
    // virtual void sendCurrentState(SystemOpMode mode, int windowPercentage) = 0;
};

#endif // CONTROL_UNIT_LINK_H