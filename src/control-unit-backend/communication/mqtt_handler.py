"""
MQTT Communication Handler for Control Unit Backend.

This module provides MQTT client functionality for communicating with the 
temperature monitoring subsystem (ESP32). It handles connection management,
message processing, and publishes control commands like sampling frequency updates.
"""

import paho.mqtt.client as mqtt
import json
import logging
from config.config import (
    MQTT_BROKER_ADDRESS, 
    MQTT_BROKER_PORT, 
    MQTT_TOPIC_TEMP_DATA, 
    MQTT_TOPIC_TEMP_CONTROL,
    MQTT_TOPIC_ESP_STATUS
)

logger = logging.getLogger(__name__)


class MqttHandler:
    """
    Handles MQTT communication with the temperature monitoring subsystem.
    
    This class manages the MQTT client connection, processes incoming temperature
    data and ESP status messages, and sends control commands to the ESP32.
    """
    
    def __init__(self, control_logic_instance):
        """
        Initialize the MQTT handler.
        
        Args:
            control_logic_instance: Reference to the main control logic instance
                                  for processing received data and state updates.
        """
        self.control_logic = control_logic_instance
        self.client = mqtt.Client(
            client_id="control_unit_backend", 
            protocol=mqtt.MQTTv311, 
            transport="tcp"
        )
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.client.on_disconnect = self._on_disconnect
        self.connected = False

    def _on_connect(self, client, userdata, flags, rc):
        """
        Callback executed when MQTT client successfully connects to broker.
        
        Args:
            client: The MQTT client instance
            userdata: User-defined data passed to callbacks
            flags: Response flags sent by the broker
            rc: Connection result code (0 = success)
        """
        if rc == 0:
            logger.info(f"Successfully connected to MQTT Broker at {MQTT_BROKER_ADDRESS}:{MQTT_BROKER_PORT}")
            
            # Subscribe to temperature data topic
            self.client.subscribe(MQTT_TOPIC_TEMP_DATA)
            logger.info(f"Subscribed to temperature topic: {MQTT_TOPIC_TEMP_DATA}")
            
            # Subscribe to ESP status topic if configured
            if MQTT_TOPIC_ESP_STATUS:
                self.client.subscribe(MQTT_TOPIC_ESP_STATUS)
                logger.info(f"Subscribed to ESP status topic: {MQTT_TOPIC_ESP_STATUS}")
            
            self.connected = True
        else:
            logger.error(f"Failed to connect to MQTT Broker, return code {rc}")
            self.connected = False

    def _on_disconnect(self, client, userdata, rc):
        """
        Callback executed when MQTT client disconnects from broker.
        
        Args:
            client: The MQTT client instance
            userdata: User-defined data passed to callbacks
            rc: Disconnection result code
        """
        logger.warning(f"Disconnected from MQTT Broker with result code {rc}. Reconnecting will be attempted by Paho.")
        self.connected = False

    def _on_message(self, client, userdata, msg):
        """
        Callback executed when a message is received on a subscribed topic.
        
        Args:
            client: The MQTT client instance
            userdata: User-defined data passed to callbacks
            msg: The received message object containing topic and payload
        """
        try:
            payload_str = msg.payload.decode('utf-8')
            logger.debug(f"Received MQTT message on topic '{msg.topic}': {payload_str}")
            data = json.loads(payload_str)

            if msg.topic == MQTT_TOPIC_TEMP_DATA:
                self._process_temperature_data(data)
            elif msg.topic == MQTT_TOPIC_ESP_STATUS:
                self._process_esp_status(data)
            else:
                logger.debug(f"Message received on unhandled topic: {msg.topic}")

        except json.JSONDecodeError:
            logger.error(f"Failed to decode JSON from MQTT message: {msg.payload.decode('utf-8')}")
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}", exc_info=True)

    def _process_temperature_data(self, data):
        """
        Process incoming temperature data from ESP32.
        
        Args:
            data: Dictionary containing temperature data from JSON payload
        """
        if "temperature" in data:
            temperature = data["temperature"]
            logger.debug(f"Processing temperature data: {temperature}°C")
            self.control_logic.process_new_temperature(temperature)
        else:
            logger.warning(f"Received temperature data without 'temperature' field: {data}")

    def _process_esp_status(self, data):
        """
        Process ESP32 status updates.
        
        Args:
            data: Dictionary containing ESP status data from JSON payload
        """
        if "status" in data:
            esp_status = data["status"]
            logger.debug(f"Processing ESP status update: {esp_status}")
            self.control_logic.update_esp_status(esp_status, data)
        else:
            logger.warning(f"Received ESP status without 'status' field: {data}")

    def connect(self):
        """
        Establish connection to the MQTT broker.
        
        Handles connection errors gracefully and updates connection status.
        """
        try:
            self.client.connect(MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT, 60)
            logger.info(f"Attempting connection to MQTT broker at {MQTT_BROKER_ADDRESS}:{MQTT_BROKER_PORT}")
        except ConnectionRefusedError:
            logger.error(f"MQTT connection refused. Is the broker running at {MQTT_BROKER_ADDRESS}:{MQTT_BROKER_PORT}?")
            self.connected = False
        except OSError as e:
            logger.error(f"MQTT OS error during connect: {e}")
            self.connected = False
        except Exception as e:
            logger.error(f"MQTT connection failed with an unexpected error: {e}", exc_info=True)
            self.connected = False

    def start_listening_loop(self):
        """
        Start the MQTT client network loop in a separate thread.
        
        This enables the client to process incoming/outgoing messages asynchronously.
        """
        try:
            self.client.loop_start()
            logger.info("MQTT listening loop started.")
        except Exception as e:
            logger.error(f"Failed to start MQTT listening loop: {e}", exc_info=True)

    def stop_listening_loop(self):
        """
        Stop the MQTT client network loop and disconnect from broker.
        
        Ensures clean shutdown of MQTT communication.
        """
        if self.client and self.connected:
            try:
                self.client.loop_stop()
                self.client.disconnect()
                logger.info("MQTT listening loop stopped and disconnected.")
            except Exception as e:
                logger.error(f"Error stopping MQTT loop or disconnecting: {e}", exc_info=True)
        else:
            logger.info("MQTT client not connected or not initialized, no need to stop loop.")
        
        self.connected = False

    def publish_sampling_frequency(self, frequency_seconds):
        """
        Publish sampling frequency command to ESP32.
        
        Args:
            frequency_seconds: Sampling interval in seconds to send to ESP32
        """
        if self.client and self.connected:
            try:
                payload = json.dumps({"frequency": frequency_seconds})
                result = self.client.publish(MQTT_TOPIC_TEMP_CONTROL, payload, qos=1)
                logger.info(f"Published sampling frequency {frequency_seconds}s to {MQTT_TOPIC_TEMP_CONTROL}")
                return result.rc == mqtt.MQTT_ERR_SUCCESS
            except Exception as e:
                logger.error(f"Error publishing sampling frequency: {e}", exc_info=True)
                return False
        else:
            logger.warning("Cannot publish sampling frequency: MQTT client not connected or not initialized.")
            return False