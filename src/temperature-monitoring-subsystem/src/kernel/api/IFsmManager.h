// src/kernel/api/IFsmManager.h (o percorso simile)
#ifndef IFSM_MANAGER_H
#define IFSM_MANAGER_H

/**
 * @enum SystemState
 * @brief Enumerates the possible operational states of the system.
 * Each state represents a distinct phase or condition in the system's lifecycle,
 * guiding its behavior and transitions.
 */
enum SystemState
{
  /** @brief Initial state upon system startup or after a full reset.
   *  System components are being initialized, and WiFi connection is typically attempted from here. */
  STATE_INITIALIZING,

  /** @brief State where the system is actively attempting to connect to the WiFi network. */
  STATE_WIFI_CONNECTING,

  /** @brief Transient state indicating successful WiFi connection.
   *  Typically leads immediately to attempting MQTT connection. */
  STATE_WIFI_CONNECTED,

  /** @brief State where the system is actively attempting to connect to the MQTT broker.
   *  This state is entered after a successful WiFi connection. */
  STATE_MQTT_CONNECTING,

    /** @brief Main operational state where WiFi and MQTT are connected and stable.
   *  The system is ready to perform its primary tasks like sampling and sending data. */
  STATE_OPERATIONAL,
  
  /** @brief State dedicated to acquiring a new temperature reading from the sensor. */
  STATE_SAMPLING_TEMPERATURE,

  /** @brief State where the acquired temperature data is being published via MQTT. */
  STATE_SENDING_DATA,

  /** @brief Indicates a general network connectivity issue (WiFi or MQTT related).
   *  The system will typically attempt recovery procedures from this state. */
  STATE_NETWORK_ERROR,
  
  /** @brief A waiting period initiated after a network error, before re-attempting connections.
   *  This prevents rapid, continuous reconnection attempts. */
  STATE_WAIT_RECONNECT
};

class IFsmManager {
public:
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