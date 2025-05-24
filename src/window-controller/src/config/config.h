#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// --- Pin Definitions ---
const int SERVO_MOTOR_PIN = 9;
const int POTENTIOMETER_PIN = A0;
const int MODE_BUTTON_PIN = 2;

// --- LCD Configuration ---
// Per quanto riguarda i pin, la libreria LiquidCrystal_I2C (e la libreria Wire sottostante
// che gestisce la comunicazione I2C) sa già quali pin usare per SDA e SCL sulla scheda Arduino per cui
// sta compilando, nel mio caso Arduino Uno:
// SDA --> Pin A4
// SCL --> Pin A5
const int LCD_I2C_ADDRESS = 0x27; // o 0x3F
const int LCD_COLUMNS = 16;
const int LCD_ROWS = 4;

// --- Servo Configuration ---
// Questi potrebbero essere passati al costruttore dell'implementazione del servo
// o usati direttamente dall'implementazione se essa legge config.h
const int WINDOW_SERVO_MIN_ANGLE_DEGREES = 0;    // Finestra chiusa (0%)
const int WINDOW_SERVO_MAX_ANGLE_DEGREES = 90;   // Finestra aperta (100%)

// --- Button Debounce ---
const unsigned long BUTTON_DEBOUNCE_DELAY_MS = 50;

// --- Potentiometer ---
const int POT_READ_CHANGE_THRESHOLD = 5; // Variazione minima per registrare un cambiamento

// --- Serial Communication ---
const long SERIAL_COM_BAUD_RATE = 115200;
const unsigned int SERIAL_COMMAND_BUFFER_SIZE = 64;

// --- FSM & Timing ---
const unsigned long LCD_REFRESH_INTERVAL_MS = 250;

// Stati FSM (se non definiti nella classe FSM)
enum class SystemOpMode {
    INIT,
    AUTOMATIC,
    MANUAL
};

#endif // CONFIG_H