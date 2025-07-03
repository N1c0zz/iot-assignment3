#include "../api/SystemFSMImpl.h"
#include <Arduino.h>
#include "config/config.h"

SystemFSMImpl::SystemFSMImpl(ServoMotor& servo, UserInputSource& input, ControlUnitLink& serial)
    : servoMotorCtrl(servo)
    , userInputCtrl(input)
    , serialLinkCtrl(serial)
    , currentMode(SystemOpMode::INIT)
    , targetWindowPercentage(0)
    , receivedTemperature(INVALID_TEMPERATURE)
    , lastPhysicalPotReading(0)
    , systemInAlarmState(false)
{
    // Constructor intentionally minimal - initialization in setup()
}

void SystemFSMImpl::setup() {
    onEnterInit();
}

SystemOpMode SystemFSMImpl::getCurrentMode() const {
    return currentMode;
}

int SystemFSMImpl::getWindowTargetPercentage() const {
    return targetWindowPercentage;
}

float SystemFSMImpl::getCurrentTemperature() const {
    return receivedTemperature;
}

bool SystemFSMImpl::isSystemInAlarmState() const {
    return systemInAlarmState;
}

void SystemFSMImpl::run() {
    // Detect events from all sources
    FsmEvent event = checkForEvents();
    int commandValue = 0;
    
    // Process serial commands if available
    if (serialLinkCtrl.commandAvailable()) {
        String serialCommand = serialLinkCtrl.readCommand();
        if (serialCommand.length() > 0) {
            FsmEvent serialEvent;
            processSerialCommand(serialCommand, serialEvent, commandValue);
            if (serialEvent != FsmEvent::NONE) {
                event = serialEvent;  // Prioritize serial commands
            }
        }
    }

    // Main FSM state machine logic
    switch (currentMode) {
        case SystemOpMode::INIT:
            if (event == FsmEvent::BOOT_COMPLETED) {
                handleStateTransition(SystemOpMode::AUTOMATIC);
            }
            break;

        case SystemOpMode::AUTOMATIC:
            if (event == FsmEvent::MODE_BUTTON_PRESSED && !systemInAlarmState) {
                serialLinkCtrl.sendModeChangedNotification(SystemOpMode::MANUAL);
                handleStateTransition(SystemOpMode::MANUAL);
            } else if (event == FsmEvent::SERIAL_CMD_MODE_MANUAL && !systemInAlarmState) {
                handleStateTransition(SystemOpMode::MANUAL);
                serialLinkCtrl.sendAckModeChange(SystemOpMode::MANUAL);
            } else {
                doStateActionAutomatic(event, commandValue);
            }
            break;

        case SystemOpMode::MANUAL:
            if (event == FsmEvent::MODE_BUTTON_PRESSED && !systemInAlarmState) {
                serialLinkCtrl.sendModeChangedNotification(SystemOpMode::AUTOMATIC);
                handleStateTransition(SystemOpMode::AUTOMATIC);
            } else if (event == FsmEvent::SERIAL_CMD_MODE_AUTO && !systemInAlarmState) {
                handleStateTransition(SystemOpMode::AUTOMATIC);
                serialLinkCtrl.sendAckModeChange(SystemOpMode::AUTOMATIC);
            } else {
                doStateActionManual(event, commandValue);
            }
            break;
    }
}

FsmEvent SystemFSMImpl::checkForEvents() {
    // INIT state always triggers boot completion
    if (currentMode == SystemOpMode::INIT) {
        return FsmEvent::BOOT_COMPLETED;
    }

    // Check for physical button press
    if (userInputCtrl.isModeButtonPressed()) {
        return FsmEvent::MODE_BUTTON_PRESSED;
    }

    return FsmEvent::NONE;
}

void SystemFSMImpl::processSerialCommand(const String& command, FsmEvent& outEvent, int& outCmdValue) {
    outEvent = FsmEvent::NONE;
    outCmdValue = 0;

    if (command.startsWith(F("SET_POS:"))) {
        outEvent = FsmEvent::SERIAL_CMD_SET_POS;
        outCmdValue = command.substring(8).toInt();
    } else if (command.startsWith(F("TEMP:"))) {
        outEvent = FsmEvent::SERIAL_CMD_SET_TEMP;
        receivedTemperature = command.substring(5).toFloat();
    } else if (command.startsWith(F("ALARM_STATE:"))) {
        systemInAlarmState = (command.substring(12).toInt() == 1);
    } else if (command.equalsIgnoreCase(F("MODE:AUTOMATIC"))) {
        outEvent = FsmEvent::SERIAL_CMD_MODE_AUTO;
    } else if (command.equalsIgnoreCase(F("MODE:MANUAL"))) {
        outEvent = FsmEvent::SERIAL_CMD_MODE_MANUAL;
    }
}

void SystemFSMImpl::handleStateTransition(SystemOpMode newMode) {
    if (currentMode == newMode) {
        return;  // No transition needed
    }

    currentMode = newMode;

    // Execute entry actions for new state
    switch (currentMode) {
        case SystemOpMode::INIT:
            onEnterInit();
            break;
        case SystemOpMode::AUTOMATIC:
            onEnterAutomatic();
            break;
        case SystemOpMode::MANUAL:
            onEnterManual();
            break;
    }
}

void SystemFSMImpl::onEnterInit() {
    targetWindowPercentage = 0;  // Start with closed window
    servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
    receivedTemperature = INVALID_TEMPERATURE;
}

void SystemFSMImpl::onEnterAutomatic() {
    // In automatic mode, wait for remote commands to set position
    // targetWindowPercentage retains its last value
}

void SystemFSMImpl::onEnterManual() {
    // Synchronize with current potentiometer position
    int currentPotReading = userInputCtrl.getPotentiometerPercentage();
    targetWindowPercentage = currentPotReading;
    lastPhysicalPotReading = currentPotReading;
    
    // Move servo to potentiometer position
    servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
    
    // Notify Control Unit of current position
    serialLinkCtrl.sendPotentiometerValue(targetWindowPercentage);
}

void SystemFSMImpl::doStateActionAutomatic(FsmEvent event, int cmdValue) {
    if (event == FsmEvent::SERIAL_CMD_SET_POS) {
        if (cmdValue >= 0 && cmdValue <= 100) {
            targetWindowPercentage = cmdValue;
            servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
        }
    }
}

void SystemFSMImpl::doStateActionManual(FsmEvent event, int cmdValue) {
    static unsigned long lastServoUpdateTime = 0;
    unsigned long currentTime = millis();

    // Block manual controls if system is in ALARM state
    if (systemInAlarmState) {
        return;
    }
    
    // Handle serial position commands (override potentiometer)
    if (event == FsmEvent::SERIAL_CMD_SET_POS && cmdValue >= 0 && cmdValue <= 100) {
        targetWindowPercentage = cmdValue;
        servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
        lastServoUpdateTime = currentTime;
        
        // Sync potentiometer tracking
        lastPhysicalPotReading = userInputCtrl.getPotentiometerPercentage();
        return;
    }
    
    // Handle potentiometer changes
    int currentPotReading = userInputCtrl.getPotentiometerPercentage();
    int potChange = abs(currentPotReading - lastPhysicalPotReading);
    
    if (potChange >= MANUAL_PERCENTAGE_CHANGE_THRESHOLD) {
        // Update target (always for tracking)
        targetWindowPercentage = currentPotReading;
        lastPhysicalPotReading = currentPotReading;
        
        // Rate-limited servo updates
        if (currentTime - lastServoUpdateTime >= SERVO_UPDATE_INTERVAL_MS) {
            servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
            lastServoUpdateTime = currentTime;
        }
        
        // Always notify Control Unit (no rate limiting)
        serialLinkCtrl.sendPotentiometerValue(targetWindowPercentage);
    }
}