#ifndef FSM_MANAGER_IMPL_H
#define FSM_MANAGER_IMPL_H

#include "IFsmManager.h"       // Interfaccia FSM

// Includi le INTERFACCE dei moduli dipendenti
#include "../../devices/api/LedStatus.h"
#include "../../devices/api/TemperatureManager.h"
#include "../connection/api/WifiManager.h"
#include "../connection/api/MqttManager.h"
#include "../../config/config.h" // Per TEMP_SAMPLE_INTERVAL_DEFAULT_MS etc.

class FsmManagerImpl : public IFsmManager {
public:
    FsmManagerImpl(LedStatus& ledCtrl, TemperatureManager& tempCtrl, WifiManager& wifiCtrl, MqttManager& mqttCtrl);
    virtual ~FsmManagerImpl() {}

    void setup() override;
    void run() override;
    SystemState getCurrentState() const override;
    float getCurrentTemperature() const override;
    unsigned long getCurrentSamplingInterval() const override;

private:
    // Riferimenti ai moduli gestiti
    LedStatus& ledController;
    TemperatureManager& tempController;
    WifiManager& wifiController;
    MqttManager& mqttController;

    // Stato interno e variabili di temporizzazione
    SystemState _currentState;
    unsigned long _lastTempSampleTime;
    unsigned long _lastMqttAttemptTime;
    unsigned long _lastWiFiAttemptTime;
    float _currentTemperature;
    unsigned long _currentSamplingIntervalMs;

    // Metodi privati per gestire la logica di stato (opzionale, o tutto in run())
    void handleInitializingState();
    void handleWiFiConnectedState();
    void handleMqttConnectingState(unsigned long currentTime);
    void handleOperationalState(unsigned long currentTime);
    void handleSamplingTemperatureState(unsigned long currentTime);
    void handleSendingDataState();
    void handleNetworkErrorState(unsigned long currentTime);
    void handleWaitReconnectState(unsigned long currentTime);

    void checkAndUpdateSamplingInterval();
};

#endif // FSM_MANAGER_IMPL_H