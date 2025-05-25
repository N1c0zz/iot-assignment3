#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> // Required for Arduino-specific types like A0 if not implicitly included

// --- Pin Definitions ---
// These constants define the Arduino pins to which hardware components are connected.

/** @brief Digital PWM pin connected to the servo motor's signal line. */
const int SERVO_MOTOR_PIN = 9;
/** @brief Analog input pin connected to the potentiometer's wiper. */
const int POTENTIOMETER_PIN = A0;
/** @brief Digital input pin connected to the mode selection button. */
const int MODE_BUTTON_PIN = 2;

// --- LCD Configuration ---
// For I2C LCDs on standard Arduino boards (like UNO), SDA and SCL pins are fixed:
// - SDA: Pin A4
// - SCL: Pin A5
// The LiquidCrystal_I2C library automatically uses these default pins.

/** @brief I2C address of the LCD module. Common values are 0x27 or 0x3F. */
const int LCD_I2C_ADDRESS = 0x27;
/** @brief Number of columns on the LCD display. */
const int LCD_COLUMNS = 16;
/** @brief Number of rows on the LCD display. */
const int LCD_ROWS = 4;

// --- Servo Motor Configuration ---
// Defines the operational angular range of the servo motor,
// corresponding to 0% (closed) and 100% (fully open) window positions.

/** @brief Servo angle in degrees for the 0% (window closed) position. */
const int WINDOW_SERVO_MIN_ANGLE_DEGREES = 0;
/** @brief Servo angle in degrees for the 100% (window fully open) position. */
const int WINDOW_SERVO_MAX_ANGLE_DEGREES = 90;

// --- Button Debounce Configuration ---

/** @brief Debounce delay in milliseconds for the mode button to prevent multiple triggers from a single press. */
const unsigned long BUTTON_DEBOUNCE_DELAY_MS = 50;

// --- Potentiometer Configuration ---

/**
 * @brief Minimum change in the raw analog reading (0-1023) of the potentiometer
 *        to be considered a significant input change. Used if filtering raw values.
 *        Currently, filtering is done via moving average on raw, then hysteresis on percentage.
 */
const int POT_READ_CHANGE_THRESHOLD = 5; // Not actively used if only moving average + FSM hysteresis is applied

/**
 * @brief Minimum change in the calculated percentage (0-100) required in MANUAL mode
 *        before the servo position is updated. This acts as a hysteresis band to
 *        prevent servo jitter from small potentiometer fluctuations.
 */
const int MANUAL_PERCENTAGE_CHANGE_THRESHOLD = 2;

// --- Serial Communication Configuration ---

/** @brief Baud rate for serial communication with the Control Unit. */
const long SERIAL_COM_BAUD_RATE = 115200;
/** @brief Size of the buffer (in bytes) for assembling incoming serial commands. */
const unsigned int SERIAL_COMMAND_BUFFER_SIZE = 64;

// --- FSM (Finite State Machine) & Timing Configuration ---

/** @brief Interval in milliseconds at which the LCD display attempts to refresh its content. */
const unsigned long LCD_REFRESH_INTERVAL_MS = 250;

/**
 * @enum SystemOpMode
 * @brief Defines the main operational modes of the window controller system.
 *        This enum is used by the FSM to manage its current state and behavior.
 */
enum class SystemOpMode {
    INIT,       ///< Initializing state, typically transient upon boot.
    AUTOMATIC,  ///< Automatic mode: window position controlled by remote commands.
    MANUAL      ///< Manual mode: window position controlled by the local potentiometer.
};

#endif // CONFIG_H