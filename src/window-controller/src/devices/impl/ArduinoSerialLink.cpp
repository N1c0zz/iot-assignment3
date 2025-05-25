#include "../api/ArduinoSerialLink.h"

ArduinoSerialLink::ArduinoSerialLink()
    : bufferIndex(0), cmdReady(false) {
    internalBuffer[0] = '\0'; // Inizializza il buffer
}

void ArduinoSerialLink::setup(long baudRate) {
    Serial.begin(baudRate);
    // Serial.println(F("Serial Link Ready.")); // Messaggio di avvio opzionale
}

// Metodo helper privato per leggere dalla seriale e costruire un comando
void ArduinoSerialLink::processIncomingSerial() {
    if (cmdReady) return; // Non processare se un comando è già in attesa

    while (Serial.available() > 0 && !cmdReady) {
        char incomingChar = Serial.read();
        if (incomingChar == '\n' || incomingChar == '\r') { // Fine comando
            if (bufferIndex > 0) { // Abbiamo qualcosa nel buffer
                internalBuffer[bufferIndex] = '\0'; // Termina la stringa C
                pendingCommand = String(internalBuffer);
                pendingCommand.trim();
                bufferIndex = 0; // Resetta per il prossimo comando
                cmdReady = true; // Segnala che un comando è pronto
            }
        } else {
            if (bufferIndex < SERIAL_COMMAND_BUFFER_SIZE - 1) {
                internalBuffer[bufferIndex++] = incomingChar;
            } else {
                // Buffer overflow, scarta e logga errore
                Serial.println(F("ERR:CMD_OVERFLOW"));
                bufferIndex = 0; // Resetta
            }
        }
    }
}


bool ArduinoSerialLink::commandAvailable() {
    processIncomingSerial(); // Tenta di leggere e costruire un comando
    return cmdReady;
}

String ArduinoSerialLink::readCommand() {
    processIncomingSerial(); // Assicura che abbiamo processato la seriale
    if (cmdReady) {
        cmdReady = false; // Consuma il comando
        String tempCmd = pendingCommand;
        pendingCommand = ""; // Pulisci per sicurezza
        return tempCmd;
    }
    return ""; // Nessun comando pronto
}

void ArduinoSerialLink::sendModeChangedNotification(SystemOpMode newMode) {
    Serial.print(F("MODE_CHANGED:"));
    if (newMode == SystemOpMode::MANUAL) {
        Serial.println(F("MANUAL"));
    } else if (newMode == SystemOpMode::AUTOMATIC) {
        Serial.println(F("AUTOMATIC"));
    }
    // Aggiungere altri stati se necessario
}

void ArduinoSerialLink::sendAckModeChange(SystemOpMode acknowledgedMode) {
    Serial.print(F("ACK_MODE:"));
    if (acknowledgedMode == SystemOpMode::MANUAL) {
        Serial.println(F("MANUAL"));
    } else if (acknowledgedMode == SystemOpMode::AUTOMATIC) {
        Serial.println(F("AUTOMATIC"));
    }
    // Aggiungere altri stati se necessario
}

void ArduinoSerialLink::sendPotentiometerValue(int percentage)
{
    Serial.print(F("POT:"));
    Serial.println(percentage);
}