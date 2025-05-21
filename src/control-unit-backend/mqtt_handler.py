import paho.mqtt.client as mqtt
import json
import logging
from config import MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT, MQTT_TOPIC_TEMP_DATA, MQTT_TOPIC_TEMP_CONTROL

logger = logging.getLogger(__name__)

# Gestisce tutta la comunicazione con il broker MQTT per interagire con il temperature-monitoring-subsystem
class MqttHandler:
    def __init__(self, control_logic_instance):
        self.control_logic = control_logic_instance
        self.client = mqtt.Client(client_id="control_unit_backend", protocol=mqtt.MQTTv311, transport="tcp") # Creo un client MQTT
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.client.on_disconnect = self._on_disconnect
        self.connected = False

    def _on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            logger.info(f"Successfully connected to MQTT Broker at {MQTT_BROKER_ADDRESS}:{MQTT_BROKER_PORT}")
            self.client.subscribe(MQTT_TOPIC_TEMP_DATA)
            logger.info(f"Subscribed to topic: {MQTT_TOPIC_TEMP_DATA}")
            self.connected = True
        else:
            logger.error(f"Failed to connect to MQTT Broker, return code {rc}")
            self.connected = False


    def _on_disconnect(self, client, userdata, rc):
        logger.warning(f"Disconnected from MQTT Broker with result code {rc}. Reconnecting...")
        self.connected = False
        # si potrebbe aggiungere una logica di riconnessione qui, ma paho-mqtt gestisce loop_start()
        # automaticamente i tentativi di riconnessione se il broker va giù e torna su.

    def _on_message(self, client, userdata, msg):
        try:
            payload_str = msg.payload.decode('utf-8')
            logger.debug(f"Received MQTT message on topic '{msg.topic}': {payload_str}")
            data = json.loads(payload_str)
            if "temperature" in data:
                self.control_logic.process_new_temperature(data["temperature"])
            else:
                logger.warning(f"Received MQTT message without 'temperature' field: {data}")
        except json.JSONDecodeError:
            logger.error(f"Failed to decode JSON from MQTT message: {msg.payload.decode('utf-8')}")
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")

    def connect(self):
        try:
            self.client.connect(MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT, 60)
        except Exception as e:
            logger.error(f"MQTT connection failed: {e}")

    # Avvia il loop di rete del client MQTT in un thread separato (client.loop_start()).
    # Questo permette al programma principale di continuare a funzionare mentre il client
    # gestisce la comunicazione MQTT in background.
    def start_listening_loop(self):
        self.client.loop_start() # Starts a new thread for network loop
        logger.info("MQTT listening loop started.")

    # Ferma il thread di rete MQTT e disconnette il client dal broker.
    def stop_listening_loop(self):
        if self.connected:
            self.client.loop_stop() # Stops the network thread
            self.client.disconnect()
        logger.info("MQTT listening loop stopped and disconnected.")

    # Crea un messaggio JSON (es. {"frequency": 60}) e lo pubblica sul topic.
    def publish_sampling_frequency(self, frequency_seconds):
        if self.connected:
            payload = json.dumps({"frequency": frequency_seconds})
            result = self.client.publish(MQTT_TOPIC_TEMP_CONTROL, payload)
            result.wait_for_publish(timeout=5) # Aspetta conferma pubblicazione
            if result.is_published():
                logger.info(f"Published sampling frequency {frequency_seconds}s to {MQTT_TOPIC_TEMP_CONTROL}")
            else:
                logger.warning(f"Failed to publish sampling frequency to {MQTT_TOPIC_TEMP_CONTROL}")

        else:
            logger.warning("Cannot publish sampling frequency: MQTT client not connected.")