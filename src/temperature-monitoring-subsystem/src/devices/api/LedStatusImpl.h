#ifndef LED_STATUS_IMPL_H
#define LED_STATUS_IMPL_H

#include "LedStatus.h"
#include "config/config.h" // Per i PIN dei LED

/**
 * @class LedStatusImpl
 * @brief Implements LedStatus for two standard (green/red) LEDs.
 * Manages GPIO pins to provide visual feedback on system status.
 */
class LedStatusImpl : public LedStatus {
public:
  /**
   * @brief Constructor for LedStatusImpl.
   * Initializes with specified or default GPIO pins for green and red LEDs.
   * @param greenLedPin GPIO pin for the green LED. Defaults to GREEN_LED_PIN from config.
   * @param redLedPin GPIO pin for the red LED. Defaults to RED_LED_PIN from config.
   */
  LedStatusImpl(int greenLedPin = GREEN_LED_PIN, int redLedPin = RED_LED_PIN);
  /**
   * @brief Default virtual destructor.
   */
  virtual ~LedStatusImpl() {}

  void setup() override;
  void indicateOperational() override;
  void indicateNetworkError() override;
  void indicateSystemBoot() override;
  void indicateWifiConnecting() override;
  void indicateMqttConnecting() override;
  void turnLedsOff() override;

private:
  int _greenLedPin;   ///< GPIO pin assigned to the green LED.
  int _redLedPin;     ///< GPIO pin assigned to the red LED.
};

#endif // LED_STATUS_IMPL_H