#ifndef FSM_MANAGER_H
#define FSM_MANAGER_H

enum SystemState {
  STATE_INITIALIZING,
  STATE_WIFI_CONNECTING,
  STATE_WIFI_CONNECTED,         // Transitorio, per avviare MQTT
  STATE_MQTT_CONNECTING,
  STATE_OPERATIONAL,            // WiFi e MQTT OK, pronto per campionare/inviare
  STATE_SAMPLING_TEMPERATURE,
  STATE_SENDING_DATA,
  STATE_NETWORK_ERROR,          // Problema WiFi o MQTT generale
  STATE_WAIT_RECONNECT          // Pausa prima di ritentare la connessione
};

extern SystemState currentState;

#endif