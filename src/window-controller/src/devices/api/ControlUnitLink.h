#ifndef CONTROL_UNIT_LINK_H
#define CONTROL_UNIT_LINK_H

#include <Arduino.h>
#include "config/config.h"

/**
 * @class ControlUnitLink
 * @brief Abstract interface for Control Unit communication
 * 
 * This interface defines the contract for bidirectional communication
 * with the system's Control Unit. It handles both incoming commands
 * (position settings, mode changes) and outgoing notifications
 * (status updates, user input events).
 * 
 * Communication Protocol:
 * - Incoming Commands:
 *   - "SET_POS:<percentage>\\n" - Set window position (0-100%)
 *   - "TEMP:<temperature>\\n" - Update temperature reading
 *   - "MODE:AUTOMATIC\\n" - Switch to automatic mode
 *   - "MODE:MANUAL\\n" - Switch to manual mode
 * 
 * - Outgoing Messages:
 *   - "POT:<percentage>\\n" - Report potentiometer position
 *   - "MODE_CHANGED:<mode>\\n" - Notify mode change initiated locally
 *   - "ACK_MODE:<mode>\\n" - Acknowledge mode change command
 */
class ControlUnitLink {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~ControlUnitLink() = default;

    /**
     * @brief Initialize communication link
     * 
     * @param baudRate Communication baud rate
     */
    virtual void setup(long baudRate) = 0;

    /**
     * @brief Check if complete command is available
     * 
     * Checks the communication buffer for complete, newline-terminated
     * commands ready for processing. Should handle command assembly
     * from incoming data stream.
     * 
     * @return true if complete command is ready for reading
     * @return false if no complete command available
     */
    virtual bool commandAvailable() = 0;

    /**
     * @brief Read complete command from buffer
     * 
     * Retrieves and consumes the oldest complete command from
     * the internal buffer. Command is removed from buffer after reading.
     * 
     * @return Complete command string (without termination characters)
     * @return Empty string if no command available
     */
    virtual String readCommand() = 0;

    /**
     * @brief Send potentiometer value to Control Unit
     * 
     * Transmits current potentiometer position as percentage.
     * Used in MANUAL mode to notify Control Unit of user input.
     * 
     * Message format: "POT:<percentage>\\n"
     * 
     * @param percentage Current potentiometer position (0-100%)
     */
    virtual void sendPotentiometerValue(int percentage) = 0;

    /**
     * @brief Send mode change notification
     * 
     * Notifies Control Unit that system mode was changed locally
     * (via button press). This is a notification, not a request.
     * 
     * Message format: "MODE_CHANGED:<MANUAL|AUTOMATIC>\\n"
     * 
     * @param newMode The new operational mode after local change
     */
    virtual void sendModeChangedNotification(SystemOpMode newMode) = 0;

    /**
     * @brief Send mode change acknowledgment
     * 
     * Acknowledges successful processing of a mode change command
     * received from the Control Unit.
     * 
     * Message format: "ACK_MODE:<MANUAL|AUTOMATIC>\\n"
     * 
     * @param acknowledgedMode The mode that was successfully set
     */
    virtual void sendAckModeChange(SystemOpMode acknowledgedMode) = 0;
};

#endif // CONTROL_UNIT_LINK_H