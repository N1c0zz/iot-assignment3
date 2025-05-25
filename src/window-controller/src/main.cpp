/**
 * @file main.cpp
 * @brief Main program for the Smart Window Controller.
 *
 * This file contains the setup() and loop() functions for the Arduino.
 * It initializes all hardware components and the Finite State Machine (FSM),
 * and then repeatedly executes the FSM logic and updates the LCD display in the main loop.
 * The architecture uses interfaces for hardware components to promote decoupling and testability.
 */
#include <Arduino.h>
#include "config/config.h" // Main configuration file (pins, constants)

// Interface headers for component abstraction
#include "devices/api/LcdView.h"
#include "devices/api/ServoMotor.h"      // Changed from ServoMotorImpl.h to ServoMotor.h
#include "devices/api/UserInputSource.h" // Changed from ArduinoPinInput.h to UserInputSource.h
#include "devices/api/ControlUnitLink.h" // Changed from ArduinoSerialLink.h to ControlUnitLink.h

// Implementation headers (needed only here for instantiation)
#include "devices/api/ServoMotorImpl.h"    // Corrected path if api/impl structure
#include "devices/api/I2CLcdView.h"        // Corrected path
#include "devices/api/ArduinoPinInput.h"   // Corrected path
#include "devices/api/ArduinoSerialLink.h" // Corrected path

// FSM class header
#include "kernel/api/SystemFSM.h" // Assuming FSM is in kernel/api path

// --- Concrete Object Instantiation ---
// Instances of the specific hardware driver implementations.
// These are often global or static for simplicity on embedded systems like Arduino.
ServoMotorImpl actualServo(SERVO_MOTOR_PIN, WINDOW_SERVO_MIN_ANGLE_DEGREES, WINDOW_SERVO_MAX_ANGLE_DEGREES);
I2CLcdView actualLcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);
ArduinoPinInput actualUserInput(MODE_BUTTON_PIN, POTENTIOMETER_PIN, BUTTON_DEBOUNCE_DELAY_MS);
ArduinoSerialLink actualSerialLink;

// --- Pointers to Interfaces for Decoupling ---
// The rest of the system (especially the FSM) will interact with components
// through these interface pointers, not directly with the concrete implementations.
// This allows for easier changes of implementations later if needed.
ServoMotor* pServoMotor = &actualServo;
LcdView* pLcdView = &actualLcd;
UserInputSource* pUserInput = &actualUserInput;
ControlUnitLink* pSerialLink = &actualSerialLink;

// --- Finite State Machine Instance ---
// The FSM is given references to the component interfaces it needs to control or query.
SystemFSM stateMachine(*pServoMotor, *pUserInput, *pSerialLink);

/**
 * @brief Setup function, runs once at an Arduino's startup.
 * Initializes serial communication, hardware components via their interfaces,
 * and the main state machine.
 */
void setup() {
    // Initialize serial communication first for logging capabilities.
    pSerialLink->setup(SERIAL_COM_BAUD_RATE);
    Serial.println(F("--- Window Controller Booting ---")); // General boot message

    // Setup hardware components through their interface pointers.
    pServoMotor->setup();
    pLcdView->setup();      // LCD will typically show its own booting message.
    pUserInput->setup();
    // Serial link setup was already done.

    // Setup the Finite State Machine. This will transition it to its initial state (e.g., INIT).
    stateMachine.setup();

    pLcdView->displayReadyMessage(); // Display "System Ready" after all initializations.

    Serial.println(F("--- System Initialized & Ready ---"));
}

/**
 * @brief Main loop function, runs repeatedly after setup().
 * Continuously executes the FSM logic and updates the LCD display.
 */
void loop() {
    // 1. Run one cycle of the Finite State Machine.
    //    The FSM handles event detection, state transitions, and actions.
    stateMachine.run();

    // 2. Update the LCD display with current information from the FSM.
    pLcdView->update(
        (stateMachine.getCurrentMode() == SystemOpMode::AUTOMATIC), // Current mode
        stateMachine.getWindowTargetPercentage(),                   // Current window target
        stateMachine.getCurrentTemperature()                        // Current temperature
    );

    // A small delay can be added to prevent the loop from running too fast,
    // potentially conserving a tiny bit of power or giving hardware time to respond,
    // though often not strictly necessary if all operations are non-blocking.
    delay(10);
}