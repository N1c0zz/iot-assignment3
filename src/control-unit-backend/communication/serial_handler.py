# serial_handler.py
import serial
import threading
import time
import logging
from config.config import SERIAL_PORT, SERIAL_BAUDRATE, MODE_MANUAL, MODE_AUTOMATIC

logger = logging.getLogger(__name__)

# Variabile globale per un check extra, per vedere se questo file viene ricaricato
SERIAL_HANDLER_FILE_VERSION = "DEBUG_V1.0" # Cambia questo se modifichi il file

class SerialHandler:
    def __init__(self, control_logic_instance):
        logger.info(f"SerialHandler Initialized - Version: {SERIAL_HANDLER_FILE_VERSION}") # Log della versione
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
            line = "" # Inizializza line per il blocco except UnicodeDecodeError
            try:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8').strip()
                    if line:
                        logger.debug(f"Received from Arduino: '{line}'")
                        self._process_serial_data(line) # Chiamata a _process_serial_data
            except serial.SerialException as e_ser:
                logger.error(f"Serial error during listening: {e_ser}. Stopping listener.", exc_info=True) # Aggiunto exc_info
                self.is_running = False
                break # Esce dal loop while
            except UnicodeDecodeError as e_uni:
                logger.warning(f"Could not decode serial data (raw: {line if line else 'N/A'}): {e_uni}", exc_info=True) # Aggiunto exc_info
            except NameError as ne: # Cattura specificamente NameError
                logger.error(f"CRITICAL NAME_ERROR in _listen_for_data or _process_serial_data: {ne}", exc_info=True)
                # Non fermare il loop per un NameError qui, ma loggalo con forza
            except Exception as e_gen:
                logger.error(f"Unexpected error in serial listening loop: {e_gen}", exc_info=True) # Aggiunto exc_info
                # Considera se fermare il loop per errori generici: self.is_running = False
            time.sleep(0.05)
        logger.info("Serial listening thread stopped.")


    def _process_serial_data(self, data_line):
        # Inserisci un log all'inizio di questa funzione
        logger.debug(f"SERIAL_HANDLER_DEBUG: _process_serial_data called with: '{data_line}'")
        try:
            if data_line.startswith("MODE_CHANGED:"):
                # new_mode_str è locale a questo blocco if
                new_mode_str = data_line.split(":")[1].strip() # strip() è una buona aggiunta
                logger.info(f"Mode change notification from Arduino: {new_mode_str}")
                if new_mode_str == "MANUAL":
                    self.control_logic.set_mode(MODE_MANUAL)
                elif new_mode_str == "AUTOMATIC":
                    self.control_logic.set_mode(MODE_AUTOMATIC)
                else:
                    logger.warning(f"Unknown mode string '{new_mode_str}' in MODE_CHANGED data.")
            elif data_line.startswith("POT:"):
                # ... (codice POT invariato, ma puoi aggiungere try-except più specifici se vuoi) ...
                try:
                    value_str = data_line.split(":")[1]
                    self.control_logic.set_manual_window_opening(value_str)
                except IndexError:
                    logger.warning(f"Malformed POT data from Arduino: {data_line}")
                except Exception as e_pot: # Catch specifico per il blocco POT
                    logger.error(f"Error processing POT data '{data_line}': {e_pot}", exc_info=True)
            else:
                logger.debug(f"Unknown data from Arduino: {data_line}")
        except NameError as ne_proc: # Cattura NameError specificamente dentro _process_serial_data
             logger.error(f"CRITICAL NAME_ERROR in _process_serial_data internal logic: {ne_proc}", exc_info=True)
        except Exception as e_proc_gen: # Catch-all per _process_serial_data
             logger.error(f"Generic error in _process_serial_data internal logic for line '{data_line}': {e_proc_gen}", exc_info=True)


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
        percent_int = int(round(percentage * 100)) # Converti 0.0-1.0 in 0-100 intero
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