#ifndef SYSTEM_FSM_IMPL_H
#define SYSTEM_FSM_IMPL_H

#include "ISystemFSM.h"
#include "config/config.h"
#include "../../devices/api/ServoMotor.h"
#include "../../devices/api/UserInputSource.h"
#include "../../devices/api/ControlUnitLink.h"

/**
 * @enum FsmEvent
 * @brief Types of events that can trigger FSM actions or transitions
 * 
 * This enumeration defines all possible events that the FSM can
 * detect and process during its operation cycle.
 */
enum class FsmEvent {
    NONE,                   ///< No event detected this cycle
    BOOT_COMPLETED,         ///< System initialization completed
    MODE_BUTTON_PRESSED,    ///< Physical mode button was pressed
    SERIAL_CMD_SET_POS,     ///< Received "SET_POS:x" command
    SERIAL_CMD_SET_TEMP,    ///< Received "TEMP:x" command  
    SERIAL_CMD_MODE_AUTO,   ///< Received "MODE:AUTOMATIC" command
    SERIAL_CMD_MODE_MANUAL  ///< Received "MODE:MANUAL" command
};

/**
 * @class SystemFSMImpl
 * @brief Concrete FSM implementation for window controller
 * 
 * This class implements the complete window controller FSM logic
 * using dependency injection for hardware abstraction. It coordinates
 * servo control, user input processing, and serial communication
 * based on the current operational mode.
 * 
 * State Management:
 * - INIT: Transient initialization state
 * - AUTOMATIC: Remote control via serial commands
 * - MANUAL: Local control via potentiometer with hysteresis
 */
class SystemFSMImpl : public ISystemFSM {
public:
    /**
     * @brief Construct FSM with hardware dependencies
     * 
     * @param servo Reference to servo motor controller interface
     * @param input Reference to user input source interface  
     * @param serial Reference to Control Unit communication interface
     * 
     * @pre All hardware interfaces must be valid and initialized
     */
    SystemFSMImpl(ServoMotor& servo, UserInputSource& input, ControlUnitLink& serial);

    /**
     * @brief Default destructor
     */
    virtual ~SystemFSMImpl() = default;

    // ISystemFSM interface implementation
    void setup() override;
    void run() override;
    SystemOpMode getCurrentMode() const override;
    int getWindowTargetPercentage() const override;
    float getCurrentTemperature() const override;
    bool isSystemInAlarmState() const override;

private:
    ServoMotor& servoMotorCtrl;         ///< Window servo motor controller
    UserInputSource& userInputCtrl;     ///< Button and potentiometer interface
    ControlUnitLink& serialLinkCtrl;    ///< Control Unit communication interface
    
    SystemOpMode currentMode;           ///< Current operational mode
    int targetWindowPercentage;         ///< Target window position (0-100%)
    float receivedTemperature;          ///< Last temperature from Control Unit
    int lastPhysicalPotReading;         ///< Last potentiometer reading for change detection
    bool systemInAlarmState;           ///< Flag to track if system is in ALARM state

    /** @brief Sentinel value for invalid/unset temperature */
    static constexpr float INVALID_TEMPERATURE = -999.0f;
    
    /**
     * @brief Detect and classify events from all sources
     * @return Detected event type (NONE if no events)
     */
    FsmEvent checkForEvents();
    
    /**
     * @brief Parse serial command and extract event/value
     * @param command Raw command string from serial link
     * @param outEvent Detected event type (output parameter)
     * @param outCmdValue Numeric value from command (output parameter)
     */
    void processSerialCommand(const String& command, FsmEvent& outEvent, int& outCmdValue);
    
    /**
     * @brief Handle state transition and entry actions
     * @param newMode Target operational mode
     */
    void handleStateTransition(SystemOpMode newMode);

    // State entry actions
    void onEnterInit();
    void onEnterAutomatic();
    void onEnterManual();

    // State-specific event processing
    void doStateActionAutomatic(FsmEvent event, int cmdValue);
    void doStateActionManual(FsmEvent event, int cmdValue);
};

#endif // SYSTEM_FSM_IMPL_H