"""
Serial Communication Handler for Control Unit Backend.

This module provides serial communication functionality for interfacing with
the Arduino-based window controller. It handles bidirectional communication
including command sending and status/event receiving.
"""

import serial
import threading
import time
import logging
from config.config import (
    SERIAL_PORT, 
    SERIAL_BAUDRATE, 
    MODE_MANUAL, 
    MODE_AUTOMATIC
)

logger = logging.getLogger(__name__)

# Version identifier for debugging purposes
SERIAL_HANDLER_FILE_VERSION = "1.0.0"


class SerialHandler:
    """
    Handles serial communication with the Arduino window controller.
    
    This class manages the serial connection, processes incoming data from
    the Arduino (mode changes, potentiometer values), and sends commands
    (window position, temperature, system mode).
    """
    
    def __init__(self, control_logic_instance):
        """
        Initialize the serial handler.
        
        Args:
            control_logic_instance: Reference to the main control logic instance
                                  for processing received events and sending commands.
        """
        logger.info(f"SerialHandler initialized - Version: {SERIAL_HANDLER_FILE_VERSION}")
        self.control_logic = control_logic_instance
        self.ser = None
        self.is_running = False
        self.thread = None

    def connect(self):
        """
        Establish serial connection with the Arduino.
        
        Returns:
            bool: True if connection successful, False otherwise
        """
        try:
            self.ser = serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1)
            time.sleep(2)  # Allow Arduino time to reset after connection
            
            if self.ser.is_open:
                logger.info(f"Successfully connected to Arduino on {SERIAL_PORT} at {SERIAL_BAUDRATE} baud.")
                self.is_running = True
                # Start listening thread
                self.thread = threading.Thread(target=self._listen_for_data, daemon=True)
                self.thread.start()
                return True
            else:
                logger.error(f"Failed to open serial port {SERIAL_PORT}, but no exception raised.")
                return False
                
        except serial.SerialException as e:
            logger.error(f"Serial connection failed on {SERIAL_PORT}: {e}")
            self.ser = None
            return False

    def _listen_for_data(self):
        """
        Background thread function for listening to incoming serial data.
        
        Continuously reads from serial port and processes Arduino messages
        until stop_listening() is called or an error occurs.
        """
        logger.info("Serial listening thread started.")
        
        while self.is_running and self.ser and self.ser.is_open:
            line = ""
            try:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8').strip()
                    if line:
                        logger.debug(f"Received from Arduino: '{line}'")
                        self._process_serial_data(line)
                        
            except serial.SerialException as e:
                logger.error(f"Serial error during listening: {e}. Stopping listener.", exc_info=True)
                self.is_running = False
                break
                
            except UnicodeDecodeError as e:
                logger.warning(f"Could not decode serial data (raw: {line}): {e}", exc_info=True)
                
            except NameError as e:
                logger.error(f"CRITICAL NAME_ERROR in serial listening: {e}", exc_info=True)
                
            except Exception as e:
                logger.error(f"Unexpected error in serial listening loop: {e}", exc_info=True)
                
            time.sleep(0.05)  # Small delay to prevent excessive CPU usage
            
        logger.info("Serial listening thread stopped.")

    def _process_serial_data(self, data_line):
        """
        Process a single line of data received from Arduino.
        
        Args:
            data_line: String containing the received data line from Arduino
        """
        logger.debug(f"Processing serial data: '{data_line}'")
        
        try:
            if data_line.startswith("MODE_CHANGED:"):
                self._handle_mode_change_notification(data_line)
            elif data_line.startswith("POT:"):
                self._handle_potentiometer_data(data_line)
            else:
                logger.debug(f"Unknown data from Arduino: {data_line}")
                
        except NameError as e:
            logger.error(f"CRITICAL NAME_ERROR in serial data processing: {e}", exc_info=True)
        except Exception as e:
            logger.error(f"Generic error processing serial data '{data_line}': {e}", exc_info=True)

    def _handle_mode_change_notification(self, data_line):
        """
        Handle mode change notification from Arduino.
        
        Args:
            data_line: String in format "MODE_CHANGED:MANUAL" or "MODE_CHANGED:AUTOMATIC"
        """
        try:
            new_mode_str = data_line.split(":")[1].strip()
            logger.info(f"Mode change notification from Arduino: {new_mode_str}")
            
            if new_mode_str == "MANUAL":
                self.control_logic.set_mode(MODE_MANUAL)
            elif new_mode_str == "AUTOMATIC":
                self.control_logic.set_mode(MODE_AUTOMATIC)
            else:
                logger.warning(f"Unknown mode string '{new_mode_str}' in MODE_CHANGED data.")
                
        except IndexError:
            logger.warning(f"Malformed MODE_CHANGED data from Arduino: {data_line}")
        except Exception as e:
            logger.error(f"Error processing MODE_CHANGED data '{data_line}': {e}", exc_info=True)

    def _handle_potentiometer_data(self, data_line):
        """
        Handle potentiometer value data from Arduino.
        
        Args:
            data_line: String in format "POT:XX" where XX is the percentage value
        """
        try:
            value_str = data_line.split(":")[1]
            # Specify that this command comes from the potentiometer
            self.control_logic.set_manual_window_opening(value_str, source="potentiometer")
            
        except IndexError:
            logger.warning(f"Malformed POT data from Arduino: {data_line}")
        except Exception as e:
            logger.error(f"Error processing POT data '{data_line}': {e}", exc_info=True)

    def _send_command(self, command_str):
        """
        Send a command string to the Arduino via serial.
        
        Args:
            command_str: Command string to send to Arduino
            
        Returns:
            bool: True if command sent successfully, False otherwise
        """
        if self.ser and self.ser.is_open:
            try:
                # Ensure command ends with newline as required by Arduino
                if not command_str.endswith('\n'):
                    command_str += '\n'
                    
                self.ser.write(command_str.encode('utf-8'))
                logger.debug(f"Sent to Arduino: {command_str.strip()}")
                return True
                
            except serial.SerialException as e:
                logger.error(f"Serial error during send: {e}")
                return False
            except Exception as e:
                logger.error(f"Unexpected error sending serial command '{command_str.strip()}': {e}")
                return False
        else:
            logger.warning(f"Cannot send command '{command_str.strip()}': Serial port not open or not initialized.")
            return False

    def send_window_command(self, percentage):
        """
        Send window position command to Arduino.
        
        Args:
            percentage: Window opening percentage as float (0.0 to 1.0)
        """
        percent_int = int(round(percentage * 100))  # Convert 0.0-1.0 to 0-100 integer
        command = f"SET_POS:{percent_int}"
        self._send_command(command)

    def send_system_mode(self, mode_string):
        """
        Send system mode command to Arduino.
        
        Args:
            mode_string: Mode string ("AUTOMATIC" or "MANUAL")
        """
        command = f"MODE:{mode_string.upper()}"
        self._send_command(command)

    def send_temperature_to_arduino(self, temperature):
        """
        Send current temperature to Arduino for display.
        
        Args:
            temperature: Temperature value in Celsius (float)
        """
        if temperature is not None:
            command = f"TEMP:{temperature:.1f}"
            self._send_command(command)

    def send_alarm_state(self, is_alarm):
        """Send alarm state to Arduino."""
        alarm_value = 1 if is_alarm else 0
        command = f"ALARM_STATE:{alarm_value}"
        self._send_command(command)

    def stop_listening(self):
        """
        Stop the serial listening thread and close the serial connection.
        
        Ensures clean shutdown of serial communication.
        """
        self.is_running = False
        
        # Wait for listening thread to finish
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=2)
            
        # Close serial port
        if self.ser and self.ser.is_open:
            self.ser.close()
            logger.info("Serial port closed.")
            
        self.ser = None