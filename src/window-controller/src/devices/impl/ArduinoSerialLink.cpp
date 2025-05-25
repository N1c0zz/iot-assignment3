#include "../api/ArduinoSerialLink.h" // Corresponding header
#include "config/config.h"            // Potentially for SERIAL_COMMAND_BUFFER_SIZE, though already in .h

ArduinoSerialLink::ArduinoSerialLink()
    : bufferIndex(0), cmdReady(false) {
    // Ensure the internal buffer is null-terminated initially.
    if (SERIAL_COMMAND_BUFFER_SIZE > 0) { // Guard against zero-size buffer
        internalBuffer[0] = '\0';
    }
}

void ArduinoSerialLink::setup(long baudRate) {
    Serial.begin(baudRate);
}

void ArduinoSerialLink::processIncomingSerial() {
    // Do not process if a command is already buffered and waiting to be read.
    if (cmdReady) {
        return;
    }

    while (Serial.available() > 0 && !cmdReady) {
        char incomingChar = Serial.read();

        // Check for newline or carriage return as command terminator.
        if (incomingChar == '\n' || incomingChar == '\r') {
            if (bufferIndex > 0) { // If there's data in the buffer
                internalBuffer[bufferIndex] = '\0'; // Null-terminate the C-string
                pendingCommand = String(internalBuffer); // Convert to Arduino String
                pendingCommand.trim(); // Remove leading/trailing whitespace
                bufferIndex = 0;       // Reset buffer index for the next command
                cmdReady = true;       // Flag that a command is ready
            }
            // If bufferIndex is 0, it's an empty line, so ignore.
        } else {
            // Add character to buffer if there's space.
            if (bufferIndex < SERIAL_COMMAND_BUFFER_SIZE - 1) {
                internalBuffer[bufferIndex++] = incomingChar;
            } else {
                // Buffer overflow: log error, discard current buffer, and reset.
                // This prevents a partially formed, incorrect command.
                Serial.println(F("ERR:CMD_BUFFER_OVERFLOW"));
                bufferIndex = 0; // Reset to start overwriting
                internalBuffer[0] = '\0'; // Ensure it's an empty string if read now
            }
        }
    }
}

bool ArduinoSerialLink::commandAvailable() {
    processIncomingSerial(); // Attempt to read and assemble any new serial data.
    return cmdReady;
}

String ArduinoSerialLink::readCommand() {
    processIncomingSerial(); // Ensure serial input is processed before trying to read.
    if (cmdReady) {
        cmdReady = false;             // Consume the command (reset flag).
        String commandToReturn = pendingCommand;
        pendingCommand = "";          // Clear the stored command.
        return commandToReturn;
    }
    return ""; // No command was ready.
}

void ArduinoSerialLink::sendModeChangedNotification(SystemOpMode newMode) {
    Serial.print(F("MODE_CHANGED:"));
    if (newMode == SystemOpMode::MANUAL) {
        Serial.println(F("MANUAL"));
    } else if (newMode == SystemOpMode::AUTOMATIC) {
        Serial.println(F("AUTOMATIC"));
    }
}

void ArduinoSerialLink::sendAckModeChange(SystemOpMode acknowledgedMode) {
    Serial.print(F("ACK_MODE:"));
    if (acknowledgedMode == SystemOpMode::MANUAL) {
        Serial.println(F("MANUAL"));
    } else if (acknowledgedMode == SystemOpMode::AUTOMATIC) {
        Serial.println(F("AUTOMATIC"));
    }
}

void ArduinoSerialLink::sendPotentiometerValue(int percentage) {
    Serial.print(F("POT:"));
    Serial.println(percentage);
}