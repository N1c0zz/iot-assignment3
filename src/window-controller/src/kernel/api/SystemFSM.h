#ifndef SYSTEM_FSM_H
#define SYSTEM_FSM_H

#include "config/config.h"              // For SystemOpMode enum and other FSM-related constants
#include "../../devices/api/ServoMotor.h"       // Interface for servo control
#include "../../devices/api/UserInputSource.h"  // Interface for button/potentiometer input
#include "../../devices/api/ControlUnitLink.h"  // Interface for serial communication

/**
 * @enum FsmEvent
 * @brief Defines the types of events that can trigger state transitions or actions within the FSM.
 */
enum class FsmEvent {
    NONE,                   ///< No event occurred.
    BOOT_COMPLETED,         ///< System boot sequence has finished.
    MODE_BUTTON_PRESSED,    ///< The physical mode change button was pressed.
    SERIAL_CMD_SET_POS,     ///< Serial command received to set window position.
    SERIAL_CMD_SET_TEMP,    ///< Serial command received to update temperature.
    SERIAL_CMD_MODE_AUTO,   ///< Serial command received to switch to AUTOMATIC mode.
    SERIAL_CMD_MODE_MANUAL  ///< Serial command received to switch to MANUAL mode.
};

/**
 * @class SystemFSM
 * @brief Manages the state and behavior of the window controller.
 *
 * This class implements the core logic for the window controller. It depends on
 * interfaces for hardware components (servo, input, serial link), allowing for
 * flexibility in their concrete implementations.
 */
class SystemFSM {
public:
    /**
     * @brief Constructor for the SystemFSM.
     * @param servo Reference to a ServoMotor interface implementation.
     * @param input Reference to a UserInputSource interface implementation.
     * @param serial Reference to a ControlUnitLink interface implementation.
     */
    SystemFSM(ServoMotor& servo, UserInputSource& input, ControlUnitLink& serial);

    /**
     * @brief Initializes the FSM, setting its initial state.
     *        Typically called once in the main setup().
     */
    void setup();

    /**
     * @brief Executes one cycle of the FSM logic.
     *        This method should be called repeatedly in the main loop().
     *        It checks for events, processes them, handles state transitions,
     *        and performs actions based on the current state.
     */
    void run();

    /**
     * @brief Gets the current operational mode of the FSM.
     * @return The current SystemOpMode (e.g., INIT, AUTOMATIC, MANUAL).
     */
    SystemOpMode getCurrentMode() const;

    /**
     * @brief Gets the FSM's current target for the window opening percentage.
     * @return The target window position as a percentage (0-100).
     */
    int getWindowTargetPercentage() const;

    /**
     * @brief Gets the last temperature value received by the FSM.
     * @return The last known temperature, or a sentinel value if none received/valid.
     */
    float getCurrentTemperature() const;

private:
    // Dependencies injected via constructor (references to interface implementations)
    ServoMotor& servoMotorCtrl;         ///< Controls the window servo motor.
    UserInputSource& userInputCtrl;     ///< Provides button and potentiometer inputs.
    ControlUnitLink& serialLinkCtrl;    ///< Handles serial communication with the Control Unit.

    // Internal state variables
    SystemOpMode currentMode;           ///< The current operational mode of the FSM.
    int targetWindowPercentage;         ///< Desired window opening percentage (0-100).
    float receivedTemperature;          ///< Last temperature value received via serial command.

    // Internal helper methods for FSM logic
    FsmEvent checkForEvents();
    void processSerialCommand(const String& command, FsmEvent& outEvent, int& outCmdValue);
    void handleStateTransition(SystemOpMode newMode);

    // State-specific entry actions
    void onEnterInit();
    void onEnterAutomatic();
    void onEnterManual();

    // State-specific "do" actions (executed while in state)
    void doStateActionAutomatic(FsmEvent event, int cmdValue);
    void doStateActionManual(FsmEvent event, int cmdValue);
};

#endif // SYSTEM_FSM_H