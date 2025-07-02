#ifndef IFSM_MANAGER_H
#define IFSM_MANAGER_H

/**
 * @enum SystemState
 * @brief Enumerates the possible operational states of the temperature monitoring system.
 * 
 * Each state represents a distinct phase or condition in the system's lifecycle,
 * guiding its behavior and transitions during operation.
 */
enum SystemState {
    /** @brief Initial state upon system startup or after a full reset. */
    STATE_INITIALIZING,

    /** @brief State where the system is actively attempting to connect to WiFi. */
    STATE_WIFI_CONNECTING,

    /** @brief Transient state indicating successful WiFi connection. */
    STATE_WIFI_CONNECTED,

    /** @brief State where the system is actively attempting to connect to MQTT broker. */
    STATE_MQTT_CONNECTING,

    /** @brief Main operational state where WiFi and MQTT are connected and stable. */
    STATE_OPERATIONAL,

    /** @brief State dedicated to acquiring a new temperature reading from the sensor. */
    STATE_SAMPLING_TEMPERATURE,

    /** @brief State where the acquired temperature data is being published via MQTT. */
    STATE_SENDING_DATA,

    /** @brief Indicates a general network connectivity issue (WiFi or MQTT related). */
    STATE_NETWORK_ERROR,

    /** @brief Waiting period after a network error before re-attempting connections. */
    STATE_WAIT_RECONNECT
};

/**
 * @class IFsmManager
 * @brief Interface for the temperature monitoring system's finite state machine.
 * 
 * Defines the contract for managing system states, handling transitions,
 * and coordinating operations between hardware components and network communication.
 */
class IFsmManager {
public:
    /**
     * @brief Virtual destructor for proper cleanup.
     */
    virtual ~IFsmManager() {}

    /**
     * @brief Initializes the FSM and its dependent components.
     * Should be called once during system setup.
     */
    virtual void setup() = 0;

    /**
     * @brief Executes one cycle of the FSM logic.
     * This includes checking for events, handling state transitions,
     * and performing actions based on the current state.
     * Should be called repeatedly in the main loop.
     */
    virtual void run() = 0;

    /**
     * @brief Gets the current operational state of the FSM.
     * @return The current SystemState.
     */
    virtual SystemState getCurrentState() const = 0;

    /**
     * @brief Gets the last measured temperature.
     * @return The current temperature value in Celsius.
     */
    virtual float getCurrentTemperature() const = 0;

    /**
     * @brief Gets the current sampling interval.
     * @return The current sampling interval in milliseconds.
     */
    virtual unsigned long getCurrentSamplingInterval() const = 0;
};

#endif // IFSM_MANAGER_H