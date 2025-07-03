#include <Arduino.h>
#include "config/config.h"

// Interface headers for component abstraction
#include "devices/api/LcdView.h"
#include "devices/api/ServoMotor.h"
#include "devices/api/UserInputSource.h"
#include "devices/api/ControlUnitLink.h"
#include "kernel/api/ISystemFSM.h"

// Implementation headers
#include "devices/api/ServoMotorImpl.h"
#include "devices/api/I2CLcdView.h"
#include "devices/api/ArduinoPinInput.h"
#include "devices/api/ArduinoSerialLink.h"
#include "kernel/api/SystemFSMImpl.h"

// Pointers to interfaces for component decoupling
LcdView* lcdView = nullptr;
ServoMotor* servoMotor = nullptr;
UserInputSource* userInputSource = nullptr;
ControlUnitLink* controlUnitLink = nullptr;
ISystemFSM* systemFsm = nullptr;

/**
 * @brief Arduino setup function - runs once at startup
 * 
 * Initializes all hardware components and creates the main
 * FSM instance.
 */
void setup() {
    Serial.begin(SERIAL_COM_BAUD_RATE);
    while (!Serial) { ; } // Wait for serial port to be ready
    Serial.println(F("\n\n[Smart Window Controller - Arduino] System Starting..."));

    // Create instances of concrete implementations
    lcdView = new I2CLcdView(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);
    servoMotor = new ServoMotorImpl(SERVO_MOTOR_PIN, WINDOW_SERVO_MIN_ANGLE_DEGREES, WINDOW_SERVO_MAX_ANGLE_DEGREES);
    userInputSource = new ArduinoPinInput(MODE_BUTTON_PIN, POTENTIOMETER_PIN, BUTTON_DEBOUNCE_DELAY_MS);
    controlUnitLink = new ArduinoSerialLink();

    // Create FSM instance, passing references to required modules
    systemFsm = new SystemFSMImpl(*servoMotor, *userInputSource, *controlUnitLink);

    // Setup individual modules
    lcdView->setup();
    lcdView->displayBootingMessage();
    servoMotor->setup();
    userInputSource->setup();
    controlUnitLink->setup(SERIAL_COM_BAUD_RATE);

    // Setup FSM
    systemFsm->setup();

    // Display system ready message
    lcdView->displayReadyMessage();

    Serial.println(F("System initialization completed."));
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
    if (systemFsm) {
        systemFsm->run();
    }

    // Update LCD display with current system status
    if (lcdView && systemFsm) {
        lcdView->update(
            (systemFsm->getCurrentMode() == SystemOpMode::AUTOMATIC),
            systemFsm->getWindowTargetPercentage(),
            systemFsm->getCurrentTemperature()
        );
    }
}