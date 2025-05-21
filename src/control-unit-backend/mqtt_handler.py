import paho.mqtt.client as mqtt
import json
import logging
import config

logger = logging.getLogger(__name__)

class MqttHandler:
    def __init__(self, control_logic_instance):
        self.control_logic = control_logic_instance
        self.client = mqtt.Client(client_id="control_unit_backend", protocol=mqtt.MQTTv311, transport="tcp")
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.client.on_disconnect = self._on_disconnect
        self.connected = False

    def _on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            logger.info(f"Successfully connected to MQTT Broker at {config.MQTT_BROKER_ADDRESS}:{config.MQTT_BROKER_PORT}")
            self.client.subscribe(config.MQTT_TOPIC_TEMP_DATA)
            if hasattr(config, 'MQTT_TOPIC_ESP_STATUS') and config.MQTT_TOPIC_ESP_STATUS:
                 self.client.subscribe(config.MQTT_TOPIC_ESP_STATUS)
                 logger.info(f"Subscribed to topic: {config.MQTT_TOPIC_ESP_STATUS}")
            self.connected = True
        else:
            logger.error(f"Failed to connect to MQTT Broker, return code {rc}")
            self.connected = False


    def _on_disconnect(self, client, userdata, rc):
        logger.warning(f"Disconnected from MQTT Broker with result code {rc}. Reconnecting will be attempted by Paho.")
        self.connected = False


    def _on_message(self, client, userdata, msg):
        try:
            payload_str = msg.payload.decode('utf-8')
            logger.debug(f"Received MQTT message on topic '{msg.topic}': {payload_str}")
            data = json.loads(payload_str)

            if msg.topic == config.MQTT_TOPIC_TEMP_DATA:
                if "temperature" in data:
                    self.control_logic.process_new_temperature(data["temperature"])
                else:
                    logger.warning(f"Received temp data without 'temperature' field: {data}")
            elif hasattr(config, 'MQTT_TOPIC_ESP_STATUS') and msg.topic == config.MQTT_TOPIC_ESP_STATUS:
                if "status" in data:
                    esp_status = data["status"]
                    self.control_logic.update_esp_status(esp_status, data)
                else:
                    logger.warning(f"Received ESP status without 'status' field: {data}")
            else:
                logger.debug(f"Message received on unhandled topic: {msg.topic}")

        except json.JSONDecodeError:
            logger.error(f"Failed to decode JSON from MQTT message: {msg.payload.decode('utf-8')}")
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}", exc_info=True)

    def connect(self):
        try:
            self.client.connect(config.MQTT_BROKER_ADDRESS, config.MQTT_BROKER_PORT, 60)
        except ConnectionRefusedError:
            logger.error(f"MQTT connection refused. Is the broker running at {config.MQTT_BROKER_ADDRESS}:{config.MQTT_BROKER_PORT}?")
            self.connected = False
        except OSError as e: # Gestisce errori di rete più generici come "No route to host"
            logger.error(f"MQTT OS error during connect: {e}")
            self.connected = False
        except Exception as e: # Catch-all per altri errori imprevisti durante la connessione
            logger.error(f"MQTT connection failed with an unexpected error: {e}", exc_info=True)
            self.connected = False

    def start_listening_loop(self):
        try:
            self.client.loop_start() # Starts a new thread for network loop
            logger.info("MQTT listening loop started.")
        except Exception as e:
            logger.error(f"Failed to start MQTT listening loop: {e}", exc_info=True)


    def stop_listening_loop(self):
        if self.client and self.connected :
            try:
                self.client.loop_stop() # Stops the network thread
                self.client.disconnect()
                logger.info("MQTT listening loop stopped and disconnected.")
            except Exception as e:
                 logger.error(f"Error stopping MQTT loop or disconnecting: {e}", exc_info=True)
        else:
            logger.info("MQTT client not connected or not initialized, no need to stop loop.")
        self.connected = False # Assicura che sia False dopo lo stop


    def publish_sampling_frequency(self, frequency_seconds):
        if self.client and self.connected: # Controlla anche self.client
            try:
                payload = json.dumps({"frequency": frequency_seconds})
                result = self.client.publish(config.MQTT_TOPIC_TEMP_CONTROL, payload, qos=1)
                logger.info(f"Attempted to publish sampling frequency {frequency_seconds}s to {config.MQTT_TOPIC_TEMP_CONTROL}")
            except Exception as e:
                logger.error(f"Error publishing sampling frequency: {e}", exc_info=True)
        else:
            logger.warning("Cannot publish sampling frequency: MQTT client not connected or not initialized.")