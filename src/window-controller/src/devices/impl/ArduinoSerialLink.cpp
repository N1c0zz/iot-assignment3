#include "../api/ArduinoSerialLink.h"
#include "config/config.h"

ArduinoSerialLink::ArduinoSerialLink()
    : bufferIndex(0)
    , cmdReady(false)
{
    // Initialize buffer with null terminator for safety
    if (SERIAL_COMMAND_BUFFER_SIZE > 0) {
        internalBuffer[0] = '\0';
    }
}

void ArduinoSerialLink::setup(long baudRate) {
    Serial.begin(baudRate);
}

bool ArduinoSerialLink::commandAvailable() {
    processIncomingSerial();
    return cmdReady;
}

String ArduinoSerialLink::readCommand() {
    // Ensure any pending serial data is processed
    processIncomingSerial();
    
    if (cmdReady) {
        // Consume the command and reset state
        cmdReady = false;
        String commandToReturn = pendingCommand;
        pendingCommand = "";
        return commandToReturn;
    }
    
    return "";  // No command available
}

void ArduinoSerialLink::sendPotentiometerValue(int percentage) {
    Serial.print(F("POT:"));
    Serial.println(percentage);
}

void ArduinoSerialLink::sendModeChangedNotification(SystemOpMode newMode) {
    Serial.print(F("MODE_CHANGED:"));
    
    switch (newMode) {
        case SystemOpMode::MANUAL:
            Serial.println(F("MANUAL"));
            break;
        case SystemOpMode::AUTOMATIC:
            Serial.println(F("AUTOMATIC"));
            break;
        default:
            // Should not occur in normal operation
            Serial.println(F("UNKNOWN"));
            break;
    }
}

void ArduinoSerialLink::sendAckModeChange(SystemOpMode acknowledgedMode) {
    Serial.print(F("ACK_MODE:"));
    
    switch (acknowledgedMode) {
        case SystemOpMode::MANUAL:
            Serial.println(F("MANUAL"));
            break;
        case SystemOpMode::AUTOMATIC:
            Serial.println(F("AUTOMATIC"));
            break;
        default:
            // Should not occur in normal operation
            Serial.println(F("UNKNOWN"));
            break;
    }
}

void ArduinoSerialLink::processIncomingSerial() {
    // Skip processing if command already ready and waiting
    if (cmdReady) {
        return;
    }

    // Process all available incoming bytes
    while (Serial.available() > 0 && !cmdReady) {
        char incomingChar = Serial.read();

        // Check for command termination characters
        if (incomingChar == '\n' || incomingChar == '\r') {
            
            // Only process if buffer contains data
            if (bufferIndex > 0) {
                // Null-terminate the command string
                internalBuffer[bufferIndex] = '\0';
                
                // Convert to String and trim whitespace
                pendingCommand = String(internalBuffer);
                pendingCommand.trim();
                
                // Reset buffer for next command
                bufferIndex = 0;
                
                // Mark command as ready
                cmdReady = true;
            }
            // If buffer is empty, ignore the termination character
            
        } else {
            // Add character to buffer if space available
            if (bufferIndex < SERIAL_COMMAND_BUFFER_SIZE - 1) {
                internalBuffer[bufferIndex++] = incomingChar;
            } else {
                // Buffer overflow: report error and reset
                Serial.println(F("ERR:CMD_BUFFER_OVERFLOW"));
                bufferIndex = 0;
                internalBuffer[0] = '\0';
            }
        }
    }
}