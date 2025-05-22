#ifndef ARDUINO_SERIAL_LINK_H
#define ARDUINO_SERIAL_LINK_H

#include "ControlUnitLink.h"
#include <Arduino.h> // Per Serial
#include "config/config.h"  // Per SERIAL_COMMAND_BUFFER_SIZE

class ArduinoSerialLink : public ControlUnitLink {
public:
    ArduinoSerialLink();
    void setup(long baudRate) override;
    bool commandAvailable() override; // Controlla se c'è un comando completo nel buffer
    String readCommand() override;    // Restituisce il comando e pulisce il buffer interno

    void sendModeChangedNotification(SystemOpMode newMode) override;
    void sendAckModeChange(SystemOpMode acknowledgedMode) override;

private:
    char internalBuffer[SERIAL_COMMAND_BUFFER_SIZE];
    byte bufferIndex;
    String pendingCommand; // Per memorizzare un comando completo letto da commandAvailable()
    bool cmdReady;         // Flag per indicare se pendingCommand è valido

    void processIncomingSerial(); // Metodo helper per leggere e bufferizzare
};

#endif // ARDUINO_SERIAL_LINK_H