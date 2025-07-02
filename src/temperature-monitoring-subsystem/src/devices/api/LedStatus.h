#ifndef LED_STATUS_H
#define LED_STATUS_H

/**
 * @class LedStatus
 * @brief Interface for managing LED visual feedback.
 * 
 * Defines a contract for initializing and updating LEDs to reflect system states.
 * Provides visual indicators for various operational states like boot, connection attempts,
 * normal operation, and error conditions.
 */
class LedStatus {
public:
  /**
   * @brief Virtual destructor for proper cleanup.
   */
  virtual ~LedStatus() {}

  /**
   * @brief Initializes LED pins and their default state.
   * Must be called once during system startup.
   */
  virtual void setup() = 0;

  /**
   * @brief Sets LEDs to indicate normal, fully operational status.
   * Shows green LED on, red LED off.
   */
  virtual void indicateOperational() = 0;

  /**
   * @brief Sets LEDs to indicate a network or system error.
   * Shows red LED on, green LED off.
   */
  virtual void indicateNetworkError() = 0;

  /**
   * @brief Provides a visual cue for system boot-up sequence.
   * A brief blink pattern to confirm system startup.
   */
  virtual void indicateSystemBoot() = 0;

  /**
   * @brief Sets LEDs to indicate WiFi connection attempt.
   */
  virtual void indicateWifiConnecting() = 0;

  /**
   * @brief Sets LEDs to indicate MQTT connection attempt.
   */
  virtual void indicateMqttConnecting() = 0;

  /**
   * @brief Turns all managed LEDs to the OFF state.
   */
  virtual void turnLedsOff() = 0;
};

#endif // LED_STATUS_H