#ifndef CONTROL_UNIT_LINK_H
#define CONTROL_UNIT_LINK_H

#include <Arduino.h> // Per String
#include "config/config.h"  // Per SystemOpMode (se la FSM non lo espone diversamente)

class ControlUnitLink {
public:
    virtual ~ControlUnitLink() {}

    virtual void setup(long baudRate) = 0;
    virtual bool commandAvailable() = 0;
    virtual String readCommand() = 0; // Legge un comando completo se disponibile

    virtual void sendModeChangedNotification(SystemOpMode newMode) = 0;
    virtual void sendAckModeChange(SystemOpMode acknowledgedMode) = 0;
    // Potremmo aggiungere altri messaggi specifici se necessario
    // virtual void sendCurrentState(SystemOpMode mode, int windowPercentage) = 0;
};

#endif // CONTROL_UNIT_LINK_H