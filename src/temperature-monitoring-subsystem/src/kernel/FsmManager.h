#ifndef FSM_MANAGER_H
#define FSM_MANAGER_H

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

/**
 * @brief Global variable holding the current state of the Finite State Machine.
 * This variable is defined and managed in `main.cpp` and drives the system's
 * logic flow based on the values in the SystemState enum.
 */
extern SystemState currentState;

#endif