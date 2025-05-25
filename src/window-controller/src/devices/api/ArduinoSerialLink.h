#ifndef ARDUINO_SERIAL_LINK_H
#define ARDUINO_SERIAL_LINK_H

#include "ControlUnitLink.h" // The interface this class implements
#include <Arduino.h>         // For Serial object and String
#include "config/config.h"   // For SERIAL_COMMAND_BUFFER_SIZE

/**
 * @class ArduinoSerialLink
 * @brief Implements the ControlUnitLink interface for serial communication
 *        via Arduino's HardwareSerial.
 *
 * This class handles buffering of incoming serial commands and formatting
 * of outgoing messages according to the defined protocol.
 */
class ArduinoSerialLink : public ControlUnitLink {
public:
    /**
     * @brief Constructor for ArduinoSerialLink.
     */
    ArduinoSerialLink();

    /**
     * @brief Initializes the Arduino HardwareSerial port.
     * @param baudRate The baud rate for the serial communication.
     */
    void setup(long baudRate) override;

    /**
     * @brief Checks if a complete newline-terminated command has been received
     *        and is waiting in the internal buffer.
     * @return True if a command is ready, false otherwise.
     */
    bool commandAvailable() override;

    /**
     * @brief Retrieves the oldest complete command from the internal buffer.
     *        The command is consumed after being read.
     * @return The command string. Returns an empty string if no command was ready.
     */
    String readCommand() override;

    /**
     * @brief Sends the potentiometer's current percentage value over serial.
     *        Format: "POT:<percentage>\n"
     * @param percentage The potentiometer value (0-100).
     */
    void sendPotentiometerValue(int percentage) override;

    /**
     * @brief Sends a "MODE_CHANGED" notification over serial.
     *        Format: "MODE_CHANGED:MANUAL\n" or "MODE_CHANGED:AUTOMATIC\n"
     * @param newMode The system's new operational mode.
     */
    void sendModeChangedNotification(SystemOpMode newMode) override;

    /**
     * @brief Sends an "ACK_MODE" acknowledgement over serial.
     *        Format: "ACK_MODE:MANUAL\n" or "ACK_MODE:AUTOMATIC\n"
     * @param acknowledgedMode The mode that was successfully set.
     */
    void sendAckModeChange(SystemOpMode acknowledgedMode) override;

private:
    char internalBuffer[SERIAL_COMMAND_BUFFER_SIZE]; ///< Buffer for assembling incoming serial data.
    byte bufferIndex;                                ///< Current position in internalBuffer.
    String pendingCommand;                           ///< Stores a fully assembled command.
    bool cmdReady;                                   ///< Flag indicating if pendingCommand holds a valid command.

    /**
     * @brief Internal helper method to process incoming bytes from the serial port
     *        and assemble them into commands in the pendingCommand buffer.
     */
    void processIncomingSerial();
};

#endif // ARDUINO_SERIAL_LINK_H