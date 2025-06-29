#include "../api/SystemFSMImpl.h"
#include <Arduino.h> // For millis(), Serial (for potential future debug), String, abs()

SystemFSMImpl::SystemFSMImpl(ServoMotor& servo, UserInputSource& input, ControlUnitLink& serial)
    : servoMotorCtrl(servo), // Store reference to servo controller
      userInputCtrl(input),   // Store reference to user input source
      serialLinkCtrl(serial), // Store reference to serial link
      currentMode(SystemOpMode::INIT),
      targetWindowPercentage(0),
      receivedTemperature(-999.0f) // Initialize with a sentinel value for invalid temperature
{
    // Constructor body can be empty if all initialization is done via initializer list
    // or in the setup() method.
}

void SystemFSMImpl::setup() {
    // The FSM starts in the INIT state.
    // Actions to be performed upon entering INIT are called here.
    // Individual hardware components (servo, input, serial) are assumed
    // to be set up externally before this FSM setup is called.
    this->onEnterInit();
}

SystemOpMode SystemFSMImpl::getCurrentMode() const {
    return currentMode;
}

int SystemFSMImpl::getWindowTargetPercentage() const {
    return targetWindowPercentage;
}

float SystemFSMImpl::getCurrentTemperature() const {
    return receivedTemperature;
}

FsmEvent SystemFSMImpl::checkForEvents() {
    // In the INIT state, the primary event expected is BOOT_COMPLETED.
    if (currentMode == SystemOpMode::INIT) {
        return FsmEvent::BOOT_COMPLETED;
    }

    // Check for a physical button press (debounced).
    if (userInputCtrl.isModeButtonPressed()) {
        return FsmEvent::MODE_BUTTON_PRESSED;
    }

    // If a serial command is available, it will be processed by processSerialCommand,
    // which will then determine the specific FsmEvent.
    // Returning NONE here means checkForEvents itself doesn't fully parse serial commands,
    // but signals that serial input should be checked.
    if (serialLinkCtrl.commandAvailable()) {
        return FsmEvent::NONE; // Signal to check serial, actual event determined later
    }

    return FsmEvent::NONE; // No other direct events detected this cycle.
}

void SystemFSMImpl::processSerialCommand(const String& command, FsmEvent& outEvent, int& outCmdValue) {
    outEvent = FsmEvent::NONE; // Default to no specific event from this command
    outCmdValue = 0;           // Default command value

    if (command.startsWith(F("SET_POS:"))) {
        outEvent = FsmEvent::SERIAL_CMD_SET_POS;
        outCmdValue = command.substring(8).toInt();
    } else if (command.startsWith(F("TEMP:"))) {
        outEvent = FsmEvent::SERIAL_CMD_SET_TEMP;
        // Temperature value is directly updated here for simplicity.
        // The event signals that a temp update occurred.
        receivedTemperature = command.substring(5).toFloat();
    } else if (command.equalsIgnoreCase(F("MODE:AUTOMATIC"))) {
        outEvent = FsmEvent::SERIAL_CMD_MODE_AUTO;
    } else if (command.equalsIgnoreCase(F("MODE:MANUAL"))) {
        outEvent = FsmEvent::SERIAL_CMD_MODE_MANUAL;
    }
    // Unrecognized commands result in FsmEvent::NONE.
}

void SystemFSMImpl::run() {
    FsmEvent event = checkForEvents(); // Check for hardware/timer events
    int commandValue = 0;              // To store value from SET_POS command
    String serialCommandString = "";

    // If checkForEvents indicated potential serial data, or if we always check:
    if (serialLinkCtrl.commandAvailable()) {
        serialCommandString = serialLinkCtrl.readCommand();
        if (serialCommandString.length() > 0) {
            // A serial command was read, parse it to determine the specific event
            // This might override 'event' if it was NONE or another event.
            // The order of precedence might matter if multiple events occur "simultaneously".
            FsmEvent serialEvent; // Temporary for the parsed serial event
            processSerialCommand(serialCommandString, serialEvent, commandValue);
            if(serialEvent != FsmEvent::NONE) { // If serial command was valid
                event = serialEvent; // Prioritize valid serial command event
            }
        }
    }

    // Main state transition and action logic
    switch (currentMode) {
        case SystemOpMode::INIT:
            // In INIT state, only BOOT_COMPLETED event triggers a transition.
            // No "doStateAction" for INIT as it's transient.
            if (event == FsmEvent::BOOT_COMPLETED) {
                handleStateTransition(SystemOpMode::AUTOMATIC);
            }
            break;

        case SystemOpMode::AUTOMATIC:
            if (event == FsmEvent::MODE_BUTTON_PRESSED) {
                //Prima invia il cambio di modalità
                serialLinkCtrl.sendModeChangedNotification(SystemOpMode::MANUAL);
                
                // Poi effettua la transizione (che includerà onEnterManual)
                handleStateTransition(SystemOpMode::MANUAL);
            } else if (event == FsmEvent::SERIAL_CMD_MODE_MANUAL) {
                handleStateTransition(SystemOpMode::MANUAL);
                serialLinkCtrl.sendAckModeChange(SystemOpMode::MANUAL);
            } else {
                doStateActionAutomatic(event, commandValue);
            }
            break;

        case SystemOpMode::MANUAL:
            if (event == FsmEvent::MODE_BUTTON_PRESSED) {
                // Prima invia il cambio di modalità  
                serialLinkCtrl.sendModeChangedNotification(SystemOpMode::AUTOMATIC);
                
                // Poi effettua la transizione
                handleStateTransition(SystemOpMode::AUTOMATIC);
            } else if (event == FsmEvent::SERIAL_CMD_MODE_AUTO) {
                handleStateTransition(SystemOpMode::AUTOMATIC);
                serialLinkCtrl.sendAckModeChange(SystemOpMode::AUTOMATIC);
            } else {
                doStateActionManual(event, commandValue);
            }
            break;
    }
}

void SystemFSMImpl::handleStateTransition(SystemOpMode newMode) {
    // No transition if already in the target mode.
    if (currentMode == newMode) {
        return;
    }

    currentMode = newMode; // Update to the new mode

    // Execute entry actions for the new mode.
    switch (currentMode) {
        case SystemOpMode::INIT:
            // Should not typically transition back to INIT after boot.
            onEnterInit();
            break;
        case SystemOpMode::AUTOMATIC:
            onEnterAutomatic();
            break;
        case SystemOpMode::MANUAL:
            onEnterManual();
            break;
    }
}

// --- State Entry Actions ---
void SystemFSMImpl::onEnterInit() {
    targetWindowPercentage = 0; // Default to closed window
    servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
    receivedTemperature = -999.0f; // Reset/invalidate temperature
}

void SystemFSMImpl::onEnterAutomatic() {
    // When entering AUTOMATIC mode, the window position is dictated by the Control Unit.
    // The FSM doesn't change the servo position here; it waits for a SET_POS command.
    // The 'targetWindowPercentage' retains its last value, which might be from MANUAL mode
    // or a previous AUTOMATIC setting. This is usually fine, as a new SET_POS is expected.
}

void SystemFSMImpl::onEnterManual() {
    // Quando entriamo in modalità MANUAL, dobbiamo sincronizzare
    // il tracking del potenziometro con la sua posizione fisica attuale
    
    // Leggi la posizione corrente del potenziometro
    int currentPotReading = userInputCtrl.getPotentiometerPercentage();
    
    // Imposta il target alla posizione corrente del potenziometro
    targetWindowPercentage = currentPotReading;
    
    // CRUCIALE: Sincronizza il tracking con la posizione attuale
    lastPhysicalPotReading = currentPotReading;
    
    // Muovi il servo alla posizione del potenziometro
    servoMotorCtrl.setPositionPercentage(targetWindowPercentage);

    serialLinkCtrl.sendPotentiometerValue(targetWindowPercentage);
}

// --- State "Do" Actions (performed while in state) ---
void SystemFSMImpl::doStateActionAutomatic(FsmEvent event, int cmdValue) {
    // In AUTOMATIC mode, primarily respond to SET_POS commands.
    if (event == FsmEvent::SERIAL_CMD_SET_POS) {
        if (cmdValue >= 0 && cmdValue <= 100) { // Validate percentage
            targetWindowPercentage = cmdValue;
            servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
        }
    }
    // Temperature updates (from SERIAL_CMD_SET_TEMP) are handled by processSerialCommand updating receivedTemperature.
}

void SystemFSMImpl::doStateActionManual(FsmEvent event, int cmdValue) {
    // Variabile statica per il rate limiting del servo
    static unsigned long lastServoUpdateTime = 0;
    const unsigned long SERVO_UPDATE_INTERVAL_MS = 50; // Massimo 20 aggiornamenti/secondo
    
    // Leggi sempre la posizione corrente del potenziometro
    int currentPotReading = userInputCtrl.getPotentiometerPercentage();
    unsigned long currentTime = millis();
    
    // Gestisci comando seriale SET_POS (ha precedenza e bypassa il rate limiting)
    if (event == FsmEvent::SERIAL_CMD_SET_POS && cmdValue >= 0 && cmdValue <= 100) {
        targetWindowPercentage = cmdValue;
        servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
        lastServoUpdateTime = currentTime; // Aggiorna il timer
        
        // Sincronizza il tracking del potenziometro
        lastPhysicalPotReading = currentPotReading;
        
        Serial.print("MANUAL: Serial SET_POS to ");
        Serial.println(cmdValue);
        return;
    }
    
    // Gestisci movimento del potenziometro
    int potChange = abs(currentPotReading - lastPhysicalPotReading);
    if (potChange >= MANUAL_PERCENTAGE_CHANGE_THRESHOLD) {
        
        // Aggiorna sempre il target (per il tracking)
        targetWindowPercentage = currentPotReading;
        lastPhysicalPotReading = currentPotReading;
        
        // Muovi il servo SOLO se è passato abbastanza tempo dall'ultimo aggiornamento
        if (currentTime - lastServoUpdateTime >= SERVO_UPDATE_INTERVAL_MS) {
            servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
            lastServoUpdateTime = currentTime;
            
            Serial.print("MANUAL: Servo updated to ");
            Serial.println(targetWindowPercentage);
        }
        
        // Informa sempre il Control Unit (senza rate limiting)
        serialLinkCtrl.sendPotentiometerValue(targetWindowPercentage);
    }
}