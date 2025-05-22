#ifndef SYSTEM_FSM_H
#define SYSTEM_FSM_H

#include "config/config.h"         // Per SystemOpMode, e altre costanti FSM se necessarie
#include "../../devices/api/servoMotor.h"    // Interfaccia
#include "../../devices/api/UserInputSource.h"// Interfaccia
#include "../../devices/api/ControlUnitLink.h"// Interfaccia
// Non includiamo LcdView qui, la FSM prenderà decisioni, il main aggiornerà l'LCD
// oppure la FSM potrebbe restituire dati per l'LCD.
// Per ora, la FSM si concentra sulla logica di stato e controllo diretto.

// Eventi che la FSM può processare
enum class FsmEvent {
    NONE,
    BOOT_COMPLETED,
    MODE_BUTTON_PRESSED,
    SERIAL_CMD_SET_POS,
    SERIAL_CMD_SET_TEMP,
    SERIAL_CMD_MODE_AUTO,
    SERIAL_CMD_MODE_MANUAL
};

class SystemFSM {
public:
    SystemFSM(servoMotor& servo, UserInputSource& input, ControlUnitLink& serial);

    void setup();
    void run(); // Esegue un ciclo della FSM

    SystemOpMode getCurrentMode() const;
    int getWindowTargetPercentage() const; // La FSM tiene traccia della posizione desiderata
    float getCurrentTemperature() const;   // La FSM memorizza la temperatura ricevuta

private:
    // Riferimenti ai componenti esterni
    servoMotor& servoMotorCtrl;
    UserInputSource& userInputCtrl;
    ControlUnitLink& serialLinkCtrl;

    SystemOpMode currentMode;
    int targetWindowPercentage; // Posizione desiderata della finestra (0-100)
    float receivedTemperature;  // Ultima temperatura ricevuta

    // Metodi privati per la logica interna
    FsmEvent checkForEvents();
    void processSerialCommand(const String& command, FsmEvent& currentEvent, int& cmdValue);
    void handleStateTransition(SystemOpMode newMode);

    // Azioni specifiche per stato
    void onEnterInit();
    void onEnterAutomatic();
    void onEnterManual();

    void doStateActionInit();
    void doStateActionAutomatic(FsmEvent event, int cmdValue);
    void doStateActionManual(FsmEvent event, int cmdValue);
};

#endif // SYSTEM_FSM_H