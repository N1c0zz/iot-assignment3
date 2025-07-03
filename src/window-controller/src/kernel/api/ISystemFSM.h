#ifndef ISYSTEM_FSM_H
#define ISYSTEM_FSM_H

#include "config/config.h"

/**
 * @class ISystemFSM
 * @brief Abstract interface for window controller state management
 * 
 * This interface defines the contract for the window controller's
 * Finite State Machine (FSM), which manages system behavior, state
 * transitions, and event handling.
 */
class ISystemFSM {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~ISystemFSM() = default;

    /**
     * @brief Initialize the FSM and set initial state
     * 
     * Prepares the FSM for operation by setting initial state
     * and performing any necessary setup operations.
     */
    virtual void setup() = 0;

    /**
     * @brief Execute one FSM cycle
     */
    virtual void run() = 0;

    /**
     * @brief Get current operational mode
     * 
     * Returns the current operational mode of the system,
     * which determines the primary control source for window positioning.
     * 
     * @return Current SystemOpMode (INIT, AUTOMATIC, or MANUAL)
     */
    virtual SystemOpMode getCurrentMode() const = 0;

    /**
     * @brief Get current window target position
     * 
     * Returns the current target position for the window as
     * determined by the FSM based on the current operational mode.
     * 
     * In AUTOMATIC mode: Set by remote commands from Control Unit
     * In MANUAL mode: Set by local potentiometer position
     * 
     * @return Target window position as percentage (0-100)
     */
    virtual int getWindowTargetPercentage() const = 0;

    /**
     * @brief Get last received temperature value
     * 
     * Returns the most recent temperature value received from
     * the Control Unit via serial communication.
     * 
     * @return Last known temperature in Celsius
     * @return Sentinel value (negative) if no valid temperature received
     */
    virtual float getCurrentTemperature() const = 0;

    /**
     * @brief Check if system is currently in ALARM state
     * 
     * @return true if system is in ALARM state, false otherwise
     */
    virtual bool isSystemInAlarmState() const = 0;
};

#endif // ISYSTEM_FSM_H