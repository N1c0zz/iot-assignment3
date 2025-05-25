# serial_handler.py
import serial
import threading
import time
import logging
from config.config import SERIAL_PORT, SERIAL_BAUDRATE, MODE_MANUAL, MODE_AUTOMATIC

logger = logging.getLogger(__name__)

class SerialHandler:
    def __init__(self, control_logic_instance):
        self.control_logic = control_logic_instance
        self.ser = None
        self.is_running = False
        self.thread = None

    def connect(self):
        try:
            self.ser = serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=1)
            time.sleep(2) # Give Arduino time to reset after connection
            if self.ser.is_open:
                logger.info(f"Successfully connected to Arduino on {SERIAL_PORT} at {SERIAL_BAUDRATE} baud.")
                self.is_running = True
                self.thread = threading.Thread(target=self._listen_for_data, daemon=True)
                self.thread.start()
                return True
            else:
                logger.error(f"Failed to open serial port {SERIAL_PORT}, but no exception raised.")
                return False
        except serial.SerialException as e:
            logger.error(f"Serial connection failed on {SERIAL_PORT}: {e}")
            self.ser = None # Ensure ser is None if connection fails
            return False

    def _listen_for_data(self):
        logger.info("Serial listening thread started.")
        while self.is_running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8').strip()
                    if line:
                        logger.debug(f"Received from Arduino: '{line}'")
                        self._process_serial_data(line)
            except serial.SerialException as e:
                logger.error(f"Serial error during listening: {e}. Stopping listener.")
                self.is_running = False
                break
            except UnicodeDecodeError:
                logger.warning(f"Could not decode serial data: {line}")
            except Exception as e:
                logger.error(f"Unexpected error in serial listening loop: {e}")
            time.sleep(0.05) # Small delay to prevent high CPU usage
        logger.info("Serial listening thread stopped.")


    def _process_serial_data(self, data_line):
        if data_line.startswith("MODE_CHANGED:"):
            new_mode_str = data_line.split(":")[1]
            logger.info(f"Mode change notification from Arduino: {new_mode_str}")
            if new_mode_str == "MANUAL":
                self.control_logic.set_mode(MODE_MANUAL) # Assumendo che set_mode gestisca il non cambiare se già in quel modo
            elif new_mode_str == "AUTOMATIC":
                self.control_logic.set_mode(MODE_AUTOMATIC)
        elif data_line.startswith("POT:"):
            try:
                value_str = data_line.split(":")[1]
                # Qui value_str è la percentuale 0-100 letta dal potenziometro
                # La logica in control_logic.set_manual_window_opening si aspetta una stringa numerica
                self.control_logic.set_manual_window_opening(value_str)
            except IndexError:
                logger.warning(f"Malformed POT data from Arduino: {data_line}")
            except Exception as e:
                logger.error(f"Error processing POT data '{data_line}': {e}")
        else:
            logger.debug(f"Unknown data from Arduino: {data_line}")


    def _send_command(self, command_str):
        if self.ser and self.ser.is_open:
            try:
                # Aggiungi newline se non presente, come richiesto dall'Arduino
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

    def send_window_command(self, percentage): # percentage 0.0 to 1.0
        percent_int = int(percentage * 100) # Converti 0.0-1.0 in 0-100 intero
        command = f"SET_POS:{percent_int}" # Invia la percentuale
        self._send_command(command)

    def send_system_mode(self, mode_string): # mode_string è "AUTOMATIC" o "MANUAL"
        command = f"MODE:{mode_string.upper()}" # Assicura maiuscolo se FSM è case-sensitive
        self._send_command(command)

    def send_temperature_to_arduino(self, temperature):
        if temperature is not None:
            command = f"TEMP:{temperature:.1f}"
            self._send_command(command)

    def stop_listening(self):
        self.is_running = False
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=2) # Wait for thread to finish
        if self.ser and self.ser.is_open:
            self.ser.close()
            logger.info("Serial port closed.")
        self.ser = None