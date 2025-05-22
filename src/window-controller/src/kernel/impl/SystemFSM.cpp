#include "../api/SystemFSM.h"
#include <Arduino.h> // Per millis() e Serial (debug)

SystemFSM::SystemFSM(servoMotor& servo, UserInputSource& input, ControlUnitLink& serial)
    : servoMotorCtrl(servo),
      userInputCtrl(input),
      serialLinkCtrl(serial),
      currentMode(SystemOpMode::INIT),
      targetWindowPercentage(0),
      receivedTemperature(-999.0f) {}

void SystemFSM::setup() {
    // La FSM inizia nello stato INIT. Le azioni di ingresso verranno eseguite nel primo run().
    // I setup dei singoli componenti (servo, input, serial) sono fatti esternamente.
    this->onEnterInit(); // Esegui subito le azioni di ingresso per INIT
}

SystemOpMode SystemFSM::getCurrentMode() const {
    return currentMode;
}

int SystemFSM::getWindowTargetPercentage() const {
    return targetWindowPercentage;
}

float SystemFSM::getCurrentTemperature() const {
    return receivedTemperature;
}


FsmEvent SystemFSM::checkForEvents() {
    if (currentMode == SystemOpMode::INIT) { // Caso speciale per il boot
        // Qui assumiamo che il boot sia "immediato" una volta che la FSM parte
        // Se ci fossero controlli di boot più lunghi, andrebbero qui.
        return FsmEvent::BOOT_COMPLETED;
    }

    if (userInputCtrl.isModeButtonPressed()) {
        return FsmEvent::MODE_BUTTON_PRESSED;
    }

    if (serialLinkCtrl.commandAvailable()) {
        // La logica per parsare il comando e determinare l'evento specifico
        // (SET_POS, SET_TEMP, MODE_AUTO, MODE_MANUAL) avverrà in processSerialCommand
        // che viene chiamato se questo blocco è vero. Per ora, il semplice fatto che
        // un comando sia disponibile è sufficiente per triggerare il suo processamento.
        // Lo modificheremo in processSerialCommand.
        return FsmEvent::NONE; // processSerialCommand si occuperà di settare l'evento corretto
    }
    return FsmEvent::NONE;
}

void SystemFSM::processSerialCommand(const String& command, FsmEvent& outEvent, int& outCmdValue) {
    outEvent = FsmEvent::NONE; // Default a nessun evento valido dal comando
    outCmdValue = 0;

    if (command.startsWith(F("SET_POS:"))) {
        outEvent = FsmEvent::SERIAL_CMD_SET_POS;
        outCmdValue = command.substring(8).toInt();
    } else if (command.startsWith(F("TEMP:"))) {
        outEvent = FsmEvent::SERIAL_CMD_SET_TEMP;
        // La conversione a float viene fatta dopo, qui passiamo un placeholder o potremmo non passare value
        // Per ora, il float lo aggiorniamo direttamente nell'azione di stato.
        // Potremmo passare la stringa del valore se necessario.
        // Per semplicità, aggiorniamo receivedTemperature direttamente.
        receivedTemperature = command.substring(5).toFloat();
    } else if (command.equalsIgnoreCase(F("MODE:AUTOMATIC"))) {
        outEvent = FsmEvent::SERIAL_CMD_MODE_AUTO;
    } else if (command.equalsIgnoreCase(F("MODE:MANUAL"))) {
        outEvent = FsmEvent::SERIAL_CMD_MODE_MANUAL;
    }
}


void SystemFSM::run() {
    FsmEvent event = checkForEvents();
    int commandValue = 0; // Per SET_POS
    String serialCommandString = "";

    if (serialLinkCtrl.commandAvailable()) { // Se c'è un comando, leggilo e processalo
        serialCommandString = serialLinkCtrl.readCommand();
        if (serialCommandString.length() > 0) {
            // Serial.print(F("FSM RX: ")); Serial.println(serialCommandString); // Debug
            processSerialCommand(serialCommandString, event, commandValue);
            // processSerialCommand aggiorna 'event' e 'commandValue'
        }
    }

    // Logica di transizione di stato e azioni
    switch (currentMode) {
        case SystemOpMode::INIT:
            doStateActionInit(); // Può solo aspettare BOOT_COMPLETED
            if (event == FsmEvent::BOOT_COMPLETED) {
                handleStateTransition(SystemOpMode::AUTOMATIC);
            }
            break;

        case SystemOpMode::AUTOMATIC:
            if (event == FsmEvent::MODE_BUTTON_PRESSED) {
                handleStateTransition(SystemOpMode::MANUAL);
                serialLinkCtrl.sendModeChangedNotification(SystemOpMode::MANUAL);
            } else if (event == FsmEvent::SERIAL_CMD_MODE_MANUAL) {
                handleStateTransition(SystemOpMode::MANUAL);
                serialLinkCtrl.sendAckModeChange(SystemOpMode::MANUAL);
            } else {
                doStateActionAutomatic(event, commandValue);
            }
            break;

        case SystemOpMode::MANUAL:
            if (event == FsmEvent::MODE_BUTTON_PRESSED) {
                handleStateTransition(SystemOpMode::AUTOMATIC);
                serialLinkCtrl.sendModeChangedNotification(SystemOpMode::AUTOMATIC);
            } else if (event == FsmEvent::SERIAL_CMD_MODE_AUTO) {
                handleStateTransition(SystemOpMode::AUTOMATIC);
                serialLinkCtrl.sendAckModeChange(SystemOpMode::AUTOMATIC);
            } else {
                doStateActionManual(event, commandValue);
            }
            break;
    }
}

void SystemFSM::handleStateTransition(SystemOpMode newMode) {
    if (currentMode == newMode) return;

    // Serial.print(F("FSM Transition: ")); // Debug
    // Serial.print((int)currentMode); Serial.print(" -> "); Serial.println((int)newMode);

    currentMode = newMode;

    // Esegui azioni di ingresso per il nuovo stato
    switch (currentMode) {
        case SystemOpMode::INIT: // Non dovrebbe rientrare qui dopo il boot
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

// --- Azioni di Ingresso ---
void SystemFSM::onEnterInit() {
    // Serial.println(F("FSM Enter: INIT")); // Debug
    targetWindowPercentage = 0; // Finestra chiusa all'inizio
    servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
    receivedTemperature = -999.0f; // Resetta temp
}

void SystemFSM::onEnterAutomatic() {
    // Serial.println(F("FSM Enter: AUTOMATIC")); // Debug
    // Quando si entra in AUTOMATICO, la posizione della finestra
    // dovrebbe essere dettata dal Control Unit.
    // Potrebbe essere utile che il CU invii un SET_POS subito dopo un cambio di modo.
    // Per ora, la variabile targetWindowPercentage mantiene l'ultimo valore noto,
    // che verrà aggiornato da un comando SET_POS.
    // Se si arriva da MANUALE, la finestra rimane dov'era finché il CU non comanda.
}

void SystemFSM::onEnterManual() {
    // Serial.println(F("FSM Enter: MANUAL")); // Debug
    // All'ingresso in manuale, la posizione è data dal potenziometro.
    targetWindowPercentage = userInputCtrl.getPotentiometerPercentage();
    servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
}

// --- Azioni Durante lo Stato ---
void SystemFSM::doStateActionInit() {
    // Nello stato INIT, di solito si aspetta solo che il sistema sia pronto
    // o si eseguono controlli di inizializzazione.
    // L'evento BOOT_COMPLETED gestisce la transizione.
}

void SystemFSM::doStateActionAutomatic(FsmEvent event, int cmdValue) {
    if (event == FsmEvent::SERIAL_CMD_SET_POS) {
        if (cmdValue >= 0 && cmdValue <= 100) {
            targetWindowPercentage = cmdValue;
            servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
            // Serial.print(F("FSM Auto: Set Pos to ")); Serial.println(targetWindowPercentage); // Debug
        }
    }
    // L'aggiornamento di receivedTemperature da SERIAL_CMD_SET_TEMP è già avvenuto in processSerialCommand.
    // Non ci sono altre azioni attive in automatico se non reagire ai comandi.
}

void SystemFSM::doStateActionManual(FsmEvent event, int cmdValue) {
    // In manuale, leggi continuamente il potenziometro
    int potPercentage = userInputCtrl.getPotentiometerPercentage();

    // Applica una soglia per evitare jitter se il potenziometro è "rumoroso"
    // Questa logica di "cambiamento significativo" è importante
    if (abs(potPercentage - targetWindowPercentage) > (POT_READ_CHANGE_THRESHOLD / 20) ) { // Semplice threshold per il cambiamento percentuale
                                                                                       // (POT_READ_CHANGE_THRESHOLD è per il raw, qui è per la %)
                                                                                       // potremmo aver bisogno di una soglia percentuale dedicata.
                                                                                       // Per ora usiamo una piccola soglia sulla %
        targetWindowPercentage = potPercentage;
        servoMotorCtrl.setPositionPercentage(targetWindowPercentage);
        // Serial.print(F("FSM Manual: Pot Set Pos to ")); Serial.println(targetWindowPercentage); // Debug
    }
    // L'aggiornamento di receivedTemperature da SERIAL_CMD_SET_TEMP è già avvenuto.
}