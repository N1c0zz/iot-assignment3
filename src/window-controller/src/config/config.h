#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> // Required for Arduino-specific types like A0 if not implicitly included

//=============================================================================
// HARDWARE PIN DEFINITIONS
//=============================================================================

/** @brief PWM pin for servo motor control signal */
const int SERVO_MOTOR_PIN = 9;

/** @brief Analog input pin for potentiometer reading */
const int POTENTIOMETER_PIN = A0;

/** @brief Digital input pin for mode selection button (with internal pullup) */
const int MODE_BUTTON_PIN = 2;

//=============================================================================
// LCD DISPLAY CONFIGURATION
//=============================================================================

/** @brief I2C address of the LCD module (common values: 0x27, 0x3F) */
const int LCD_I2C_ADDRESS = 0x27;

/** @brief Number of character columns on the LCD display */
const int LCD_COLUMNS = 16;

/** @brief Number of character rows on the LCD display */
const int LCD_ROWS = 4;

//=============================================================================
// SERVO MOTOR CONFIGURATION
//=============================================================================

/** @brief Servo angle (degrees) for fully closed window (0%) */
const int WINDOW_SERVO_MIN_ANGLE_DEGREES = 0;

/** @brief Servo angle (degrees) for fully open window (100%) */
const int WINDOW_SERVO_MAX_ANGLE_DEGREES = 90;

/** @brief Maximum servo update interval in MANUAL mode (milliseconds) */
const unsigned long SERVO_UPDATE_INTERVAL_MS = 50;

//=============================================================================
// INPUT PROCESSING CONFIGURATION
//=============================================================================

/** @brief Button debounce delay to prevent multiple triggers (milliseconds) */
const unsigned long BUTTON_DEBOUNCE_DELAY_MS = 50;

/** 
 * @brief Minimum analog reading change threshold for potentiometer
 */
const int POT_NUM_SAMPLES = 10;

/** 
 * @brief Minimum percentage change required to update servo in MANUAL mode
 */
const int MANUAL_PERCENTAGE_CHANGE_THRESHOLD = 2;

//=============================================================================
// COMMUNICATION CONFIGURATION
//=============================================================================

/** @brief Serial communication baud rate */
const long SERIAL_COM_BAUD_RATE = 115200;

/** @brief Buffer size for incoming serial command assembly */
const unsigned int SERIAL_COMMAND_BUFFER_SIZE = 64;

//=============================================================================
// SYSTEM TIMING CONFIGURATION
//=============================================================================

/** @brief LCD display refresh interval (milliseconds) */
const unsigned long LCD_REFRESH_INTERVAL_MS = 250;

#endif // CONFIG_H