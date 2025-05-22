#include <Arduino.h>
#include "config/config.h"

// Includi le INTERFACCE che il main potrebbe usare direttamente (es. LCD)
#include "devices/api/LcdView.h"
// Includi le IMPLEMENTAZIONI per l'istanziazione
#include "devices/api/servoMotorImpl.h"
#include "devices/api/I2CLcdView.h"
#include "devices/api/ArduinoPinInput.h"
#include "devices/api/ArduinoSerialLink.h"
// Includi la classe FSM
#include "kernel/api/SystemFSM.h"

// --- Istanziazione degli oggetti concreti ---
// Questi sono gli oggetti reali. Potrebbero essere globali o creati in setup.
// Per semplicità su Arduino, li rendiamo globali o statici.
servoMotorImpl actualServo(SERVO_MOTOR_PIN, WINDOW_SERVO_MIN_ANGLE_DEGREES, WINDOW_SERVO_MAX_ANGLE_DEGREES);
I2CLcdView actualLcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);
ArduinoPinInput actualUserInput(MODE_BUTTON_PIN, POTENTIOMETER_PIN, BUTTON_DEBOUNCE_DELAY_MS);
ArduinoSerialLink actualSerialLink;

// --- Puntatori/Riferimenti alle Interfacce (usati dalla FSM e dal main) ---
// Questo permette il disaccoppiamento.
servoMotor* pservoMotor = &actualServo;
LcdView* pLcdView = &actualLcd;
UserInputSource* pUserInput = &actualUserInput;
ControlUnitLink* pSerialLink = &actualSerialLink;

// --- Istanza della FSM ---
// La FSM riceve i riferimenti alle interfacce dei componenti che controllerà o userà.
SystemFSM stateMachine(*pservoMotor, *pUserInput, *pSerialLink);


void setup() {
    // Inizializza prima la comunicazione seriale per il logging
    pSerialLink->setup(SERIAL_COM_BAUD_RATE);
    Serial.println(F("--- Window Controller Booting (OOP Advanced) ---"));

    // Setup dei componenti tramite le loro interfacce/oggetti
    pservoMotor->setup();
    pLcdView->setup(); // Mostrerà "Booting..."
    pUserInput->setup();
    // pSerialLink->setup() è già stato fatto

    // Setup della FSM
    stateMachine.setup(); // La FSM entrerà nello stato INIT

    pLcdView->displayReadyMessage(); // Mostra "System Ready" e pulisce per la FSM

    Serial.println(F("--- System Initialized & Ready ---"));
}

void loop() {
    // 1. Esegui la logica della FSM
    // La FSM internamente leggerà input (tramite pUserInput),
    // processerà comandi seriali (tramite pSerialLink),
    // e comanderà il servo (tramite pservoMotor).
    stateMachine.run();

    // 2. Aggiorna il display LCD
    // L'LCD viene aggiornato con i dati correnti mantenuti o esposti dalla FSM.
    pLcdView->update(
        (stateMachine.getCurrentMode() == SystemOpMode::AUTOMATIC),
        stateMachine.getWindowTargetPercentage(), // O pservoMotor->getCurrentPercentage() se preferito
        stateMachine.getCurrentTemperature()
    );

    // Non c'è molto altro da fare qui, la FSM e i moduli gestiscono la complessità.
    // Un piccolo delay potrebbe essere utile se il loop è troppo veloce e non ci sono
    // altre operazioni che richiedono tempo, ma con la logica dell'LCD che si aggiorna
    // a intervalli, potrebbe non essere necessario.
    // delay(10); // Opzionale, per dare respiro
}