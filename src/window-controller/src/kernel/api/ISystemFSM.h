#ifndef ISYSTEM_FSM_H
#define ISYSTEM_FSM_H

#include "config/config.h" // Per SystemOpMode enum

/**
 * @file ISystemFSM.h
 * @brief Defines the interface for the Window Controller's Finite State Machine.
 */

/**
 * @class ISystemFSM
 * @brief Interface for the window controller's state management logic.
 * Defines the public methods for initializing, running, and querying the FSM.
 */
class ISystemFSM {
public:
  /**
   * @brief Virtual destructor for proper cleanup.
   */
  virtual ~ISystemFSM() {}

  /**
   * @brief Initializes the FSM, setting its initial state.
   * Typically called once in the main setup().
   */
  virtual void setup() = 0;

  /**
   * @brief Executes one cycle of the FSM logic.
   * This method should be called repeatedly in the main loop().
   * It handles event detection, state transitions, and actions.
   */
  virtual void run() = 0;

  /**
   * @brief Gets the current operational mode of the FSM.
   * @return The current SystemOpMode (e.g., INIT, AUTOMATIC, MANUAL).
   */
  virtual SystemOpMode getCurrentMode() const = 0;

  /**
   * @brief Gets the FSM's current target for the window opening percentage.
   * @return The target window position as a percentage (0-100).
   */
  virtual int getWindowTargetPercentage() const = 0;

  /**
   * @brief Gets the last temperature value received and stored by the FSM.
   * @return The last known temperature, or a sentinel value if not valid.
   */
  virtual float getCurrentTemperature() const = 0;
};

#endif // ISYSTEM_FSM_H