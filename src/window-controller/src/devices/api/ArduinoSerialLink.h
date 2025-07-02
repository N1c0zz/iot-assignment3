#ifndef ARDUINO_SERIAL_LINK_H
#define ARDUINO_SERIAL_LINK_H

#include "ControlUnitLink.h"
#include <Arduino.h>
#include "config/config.h"

/**
 * @class ArduinoSerialLink
 * @brief Arduino HardwareSerial implementation of ControlUnitLink
 * 
 * This class provides concrete Control Unit communication using
 * Arduino's built-in HardwareSerial interface. It implements
 * robust command parsing with overflow protection and proper
 * message formatting according to the defined protocol.
 */
class ArduinoSerialLink : public ControlUnitLink {
public:
    /**
     * @brief Construct Arduino serial communication handler
     * 
     * Initializes internal buffers and state variables for
     * reliable command parsing and transmission.
     */
    ArduinoSerialLink();

    /**
     * @brief Default destructor
     */
    virtual ~ArduinoSerialLink() = default;

    // ControlUnitLink interface implementation
    void setup(long baudRate) override;
    bool commandAvailable() override;
    String readCommand() override;
    void sendPotentiometerValue(int percentage) override;
    void sendModeChangedNotification(SystemOpMode newMode) override;
    void sendAckModeChange(SystemOpMode acknowledgedMode) override;

private:
    /** @brief Buffer for assembling incoming serial commands */
    char internalBuffer[SERIAL_COMMAND_BUFFER_SIZE];
    
    /** @brief Current write position in internal buffer */
    byte bufferIndex;
    
    /** @brief Storage for complete, parsed command */
    String pendingCommand;
    
    /** @brief Flag indicating complete command is ready for reading */
    bool cmdReady;

    /**
     * @brief Process incoming serial data and assemble commands
     * 
     * Reads available bytes from serial port, assembles them into
     * commands in internal buffer, and detects command completion.
     * Handles buffer overflow and provides error reporting.
     */
    void processIncomingSerial();
};

#endif // ARDUINO_SERIAL_LINK_H