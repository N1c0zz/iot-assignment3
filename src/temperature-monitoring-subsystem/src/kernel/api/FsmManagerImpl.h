#ifndef FSM_MANAGER_IMPL_H
#define FSM_MANAGER_IMPL_H

#include "IFsmManager.h"
#include "../../devices/api/LedStatus.h"
#include "../../devices/api/TemperatureManager.h"
#include "../connection/api/WifiManager.h"
#include "../connection/api/MqttManager.h"
#include "../../config/config.h"

/**
 * @class FsmManagerImpl
 * @brief Implements the finite state machine for the temperature monitoring system.
 * 
 * Manages system states, coordinates operations between hardware components
 * and network communication, and handles state transitions based on events
 * and system conditions.
 */
class FsmManagerImpl : public IFsmManager {
public:
    /**
     * @brief Constructor for FsmManagerImpl.
     * @param ledCtrl Reference to LED status controller for visual feedback.
     * @param tempCtrl Reference to temperature manager for sensor readings.
     * @param wifiCtrl Reference to WiFi manager for network connectivity.
     * @param mqttCtrl Reference to MQTT manager for broker communication.
     */
    FsmManagerImpl(LedStatus& ledCtrl, TemperatureManager& tempCtrl, WifiManager& wifiCtrl, MqttManager& mqttCtrl);

    /**
     * @brief Virtual destructor.
     */
    virtual ~FsmManagerImpl() {}

    void setup() override;
    void run() override;
    SystemState getCurrentState() const override;
    float getCurrentTemperature() const override;
    unsigned long getCurrentSamplingInterval() const override;

private:
    // References to managed components
    LedStatus& ledController;                   ///< Controls LED visual feedback.
    TemperatureManager& tempController;         ///< Manages temperature sensor readings.
    WifiManager& wifiController;                ///< Handles WiFi connectivity.
    MqttManager& mqttController;                ///< Manages MQTT communication.

    // Internal state and timing variables
    SystemState _currentState;                  ///< Current FSM state.
    unsigned long _lastTempSampleTime;          ///< Timestamp of last temperature sample.
    unsigned long _lastMqttAttemptTime;         ///< Timestamp of last MQTT connection attempt.
    unsigned long _lastWiFiAttemptTime;         ///< Timestamp of last WiFi connection attempt.
    float _currentTemperature;                  ///< Last measured temperature value.
    unsigned long _currentSamplingIntervalMs;  ///< Current sampling interval in milliseconds.

    // Private methods for state-specific logic
    void handleInitializingState();
    void handleWiFiConnectedState();
    void handleMqttConnectingState(unsigned long currentTime);
    void handleOperationalState(unsigned long currentTime);
    void handleSamplingTemperatureState(unsigned long currentTime);
    void handleSendingDataState();
    void handleNetworkErrorState(unsigned long currentTime);
    void handleWaitReconnectState(unsigned long currentTime);

    /**
     * @brief Checks for and applies new sampling interval from MQTT.
     */
    void checkAndUpdateSamplingInterval();
};

#endif // FSM_MANAGER_IMPL_H