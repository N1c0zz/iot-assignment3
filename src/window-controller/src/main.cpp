#include <Arduino.h>
#include "config/config.h"

// Interface headers (for type declarations)
#include "devices/api/LcdView.h"
#include "devices/api/ServoMotor.h"
#include "devices/api/UserInputSource.h"
#include "devices/api/ControlUnitLink.h"
#include "kernel/api/ISystemFSM.h"

// Implementation headers (for object instantiation)
#include "devices/api/ServoMotorImpl.h"
#include "devices/api/I2CLcdView.h"
#include "devices/api/ArduinoPinInput.h"
#include "devices/api/ArduinoSerialLink.h"
#include "kernel/api/SystemFSMImpl.h"

// Concrete hardware implementations
ServoMotorImpl actualServo(SERVO_MOTOR_PIN, WINDOW_SERVO_MIN_ANGLE_DEGREES, WINDOW_SERVO_MAX_ANGLE_DEGREES);
I2CLcdView actualLcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);
ArduinoPinInput actualUserInput(MODE_BUTTON_PIN, POTENTIOMETER_PIN, BUTTON_DEBOUNCE_DELAY_MS);
ArduinoSerialLink actualSerialLink;

// Interface pointers for dependency injection
ServoMotor* pServoMotor = &actualServo;
LcdView* pLcdView = &actualLcd;
UserInputSource* pUserInput = &actualUserInput;
ControlUnitLink* pSerialLink = &actualSerialLink;

// FSM instance (created during setup)
ISystemFSM* pStateMachine = nullptr;

/**
 * @brief Arduino setup function - runs once at startup
 * 
 * Initializes all hardware components and creates the main
 * FSM instance with proper dependency injection.
 */
void setup() {
    // Initialize serial communication first for debugging capability
    pSerialLink->setup(SERIAL_COM_BAUD_RATE);
    Serial.println(F("--- Window Controller Starting ---"));

    // Initialize hardware components through their interfaces
    pServoMotor->setup();
    pLcdView->setup();      // Shows boot message automatically
    pUserInput->setup();

    // Create FSM with injected dependencies
    pStateMachine = new SystemFSMImpl(*pServoMotor, *pUserInput, *pSerialLink);
    pStateMachine->setup();

    // Display system ready message
    pLcdView->displayReadyMessage();

    Serial.println(F("--- System Ready ---"));
}

/**
 * @brief Arduino main loop
 * 
 * Executes the main system loop consisting of:
 * 1. FSM execution cycle (event processing, state transitions)
 * 2. Display update with current system status
 */
void loop() {
    // Execute one FSM cycle (event processing + state transitions)
    if (pStateMachine) {
        pStateMachine->run();
    }

    // Update LCD display with current system status
    if (pLcdView && pStateMachine) {
        pLcdView->update(
            (pStateMachine->getCurrentMode() == SystemOpMode::AUTOMATIC),
            pStateMachine->getWindowTargetPercentage(),
            pStateMachine->getCurrentTemperature()
        );
    }
}