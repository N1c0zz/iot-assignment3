"""
Configuration module for the Control Unit Backend.

This module contains all the configuration constants and parameters
used throughout the control unit backend system, including MQTT settings,
serial communication parameters, control logic thresholds, and API configuration.
"""

# === MQTT Configuration ===
# MQTT broker connection settings for communication with the temperature monitoring subsystem.
MQTT_BROKER_ADDRESS = "localhost"          # MQTT broker hostname or IP address
MQTT_BROKER_PORT = 1883                     # MQTT broker port
MQTT_TOPIC_TEMP_DATA = "assignment3/temperature"        # Topic for receiving temperature data from ESP32
MQTT_TOPIC_TEMP_CONTROL = "assignment3/frequency"       # Topic for sending control commands (sampling frequency) to ESP32
MQTT_TOPIC_ESP_STATUS = "assignment3/status"            # Topic for ESP32 status updates

# === Serial Communication Configuration ===
# Serial port settings for communication with the Arduino window controller.
SERIAL_PORT = "COM4"                        # Serial port identifier
SERIAL_BAUDRATE = 115200                    # Serial communication baud rate

# === Control Logic Parameters ===
# Temperature thresholds that define system state transitions.
T1_THRESHOLD = 20                           # Temperature threshold (°C) for NORMAL -> HOT transition
T2_THRESHOLD = 27                           # Temperature threshold (°C) for HOT -> TOO_HOT transition

# Data management and statistics configuration.
N_LAST_MEASUREMENTS = 10                    # Number of recent temperature measurements to keep for statistics

# Alarm system configuration.
DT_ALARM_DURATION_S = 5                     # Duration (seconds) system must remain in TOO_HOT state before triggering ALARM

# === Sampling Frequency Configuration ===
# Temperature sampling intervals sent to the ESP32 based on system state.
SAMPLING_FREQUENCY_F1_S = 60                # Low frequency sampling interval (seconds) for NORMAL state
SAMPLING_FREQUENCY_F2_S = 10                # High frequency sampling interval (seconds) for HOT/TOO_HOT states

# === Window Control Parameters ===
# Window opening percentages as floating point values (0.0 to 1.0).
WINDOW_CLOSED_PERCENTAGE = 0.0              # Fully closed window position (0%)
WINDOW_FULLY_OPEN_PERCENTAGE = 1.0          # Fully open window position (100%)

# === System Operation Modes ===
# Available operational modes for the control system.
MODE_AUTOMATIC = "AUTOMATIC"                # Automatic mode: system controls window based on temperature
MODE_MANUAL = "MANUAL"                      # Manual mode: user controls window via potentiometer or dashboard

# === System States ===
# Possible system states based on temperature readings and alarm conditions.
STATE_NORMAL = "NORMAL"                     # Normal operation: temperature below T1_THRESHOLD
STATE_HOT = "HOT"                          # Hot state: temperature between T1_THRESHOLD and T2_THRESHOLD
STATE_TOO_HOT = "TOO_HOT"                  # Too hot state: temperature above T2_THRESHOLD
STATE_ALARM = "ALARM"                      # Alarm state: system has been in TOO_HOT for DT_ALARM_DURATION_S

# === API Configuration ===
# Flask web API server configuration for dashboard communication.
API_HOST = "0.0.0.0"                       # Listen on all network interfaces
API_PORT = 5001                             # HTTP port for the Flask API server