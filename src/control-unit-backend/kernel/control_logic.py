import time
from collections import deque
import logging
from config.config import (
    T1_THRESHOLD, T2_THRESHOLD, N_LAST_MEASUREMENTS, DT_ALARM_DURATION_S,
    SAMPLING_FREQUENCY_F1_S, SAMPLING_FREQUENCY_F2_S,
    WINDOW_CLOSED_PERCENTAGE, WINDOW_FULLY_OPEN_PERCENTAGE,
    MODE_AUTOMATIC, MODE_MANUAL,
    STATE_NORMAL, STATE_HOT, STATE_TOO_HOT, STATE_ALARM
)

logger = logging.getLogger(__name__)

class ControlLogic:
    def __init__(self, mqtt_handler=None, serial_handler=None):
        self.mqtt_handler = mqtt_handler
        self.serial_handler = serial_handler

        self.current_mode = MODE_AUTOMATIC
        self.system_state = STATE_NORMAL
        self.esp_status = "UNKNOWN" # Variabile per lo stato dell'ESP (inizializzato ad UNKNOWN)
        self.esp_last_status_data = {}
        self.current_temperature = None
        self.last_n_temperatures = deque(maxlen=N_LAST_MEASUREMENTS) # lista a dimensione fissa per le ultime n temperature rilevate
        self.avg_temp = None
        self.min_temp = None
        self.max_temp = None
        self.window_opening_percentage = WINDOW_CLOSED_PERCENTAGE # 0.0 to 1.0

        self.too_hot_start_time = None
        self._initialize_state()

        logger.info(f"ControlLogic initialized. Mode: {self.current_mode}, State: {self.system_state}")

    # Monitora lo stato dell'ESP
    def update_esp_status(self, status, full_data_payload=None):
        self.esp_status = status
        if full_data_payload:
            self.esp_last_status_data = full_data_payload
        logger.info(f"ESP current status set to: {self.esp_status}")
        # Qui si potrebbero implementare logiche aggiuntive:
        # - Se status == "offline" o "unexpected_disconnect", potrei
        #   mettere il sistema in uno stato di "attesa sensore" o notificare la dashboard.
        # - Se status == "online", potrei resettare eventuali flag di errore relativi all'ESP.
        # - Aggiornare la dashboard con lo stato dell'ESP.

    # Chiamata all'avvio (o quando gli handler sono disponibili) per impostare lo stato
    # iniziale dei dispositivi esterni
    def _initialize_state(self):
        if self.mqtt_handler:
            self.mqtt_handler.publish_sampling_frequency(SAMPLING_FREQUENCY_F1_S)
        if self.serial_handler:
            self.serial_handler.send_system_mode(self.current_mode)
            # Invia la temperatura iniziale se in manuale (anche se all'inizio è AUTOMATIC,
            # questa logica è per coerenza se lo stato iniziale potesse essere MANUAL)
            if self.current_mode == MODE_MANUAL and self.current_temperature is not None:
                self.serial_handler.send_temperature_to_arduino(self.current_temperature)
            
            # Invia la posizione iniziale della finestra (calcolata in base alla modalità/stato iniziale)
            # Questa chiamata è importante perché imposta targetWindowPercentage sull'Arduino
            # tramite il comando SET_POS:X
            self.serial_handler.send_window_command(self.window_opening_percentage)

    # Chiamato da MqttHandler quando arriva una nuova temperatura dal temperature-monitoring-subsystem
    def process_new_temperature(self, temp_value):
        self.current_temperature = float(temp_value)
        self.last_n_temperatures.append(self.current_temperature)
        self._update_stats()
        logger.info(f"New temperature: {self.current_temperature}°C. Current mode: {self.current_mode}")

        if self.current_mode == MODE_AUTOMATIC:
            self._evaluate_automatic_mode()
        else: # MANUAL mode
            # Invia solo la temperatura aggiornata all'Arduino (solo se in MANUALE,
            # perché l'LCD Arduino mostra la temp solo in manuale)
            if self.serial_handler and self.current_temperature is not None:
                self.serial_handler.send_temperature_to_arduino(self.current_temperature)
            # La modalità e la posizione della finestra in manuale sono gestite
            # dall'Arduino stesso (bottone, potenziometro) o da comandi API specifici.
            # La Control Unit non le forza qui, solo notifica la temperatura.
        # Notifica sempre la dashboard (implicito, quando la dashboard fa GET /status)

    # Calcola media, minimo e massimo dalle temperature in last_n_temperatures.
    def _update_stats(self):
        if self.last_n_temperatures:
            self.avg_temp = sum(self.last_n_temperatures) / len(self.last_n_temperatures)
            self.min_temp = min(self.last_n_temperatures)
            self.max_temp = max(self.last_n_temperatures)
        else:
            self.avg_temp = None
            self.min_temp = None
            self.max_temp = None

    # Gestisce la logica della modalità automatica
    def _evaluate_automatic_mode(self):
        logger.debug(f"--- Evaluating Automatic Mode ---")
        logger.debug(f"Before eval: self.current_temperature = {self.current_temperature}, self.window_opening_percentage = {self.window_opening_percentage}")

        if self.system_state == STATE_ALARM:
            logger.info("System in ALARM state. Waiting for operator intervention.")
            # Non fare nulla finché l'allarme non viene resettato
            return

        previous_state = self.system_state
        previous_window_opening = self.window_opening_percentage
        new_sampling_freq = None

        if self.current_temperature is None:
            logger.warning("Cannot evaluate automatic mode: current_temperature is None.")
            return

        if self.current_temperature < T1_THRESHOLD:
            self.system_state = STATE_NORMAL
            self.window_opening_percentage = WINDOW_CLOSED_PERCENTAGE
            new_sampling_freq = SAMPLING_FREQUENCY_F1_S
            self.too_hot_start_time = None # Reset timer
        elif T1_THRESHOLD <= self.current_temperature <= T2_THRESHOLD:
            self.system_state = STATE_HOT
            # Proportional opening: P = 0.01 when T=T1, P=1.00 when T=T2
            # P = (T - T1) / (T2 - T1) * (1.00 - 0.01) + 0.01
            # Assicur0 che T2 > T1 per evitare divisione per zero
            if T2_THRESHOLD > T1_THRESHOLD:
                 self.window_opening_percentage = ((self.current_temperature - T1_THRESHOLD) /
                                                (T2_THRESHOLD - T1_THRESHOLD) * (WINDOW_FULLY_OPEN_PERCENTAGE - 0.01)) + 0.01
            else: # Caso limite T1=T2, apri a 0.01 se <= T1, altrimenti 1.0
                 self.window_opening_percentage = 0.01 if self.current_temperature <= T1_THRESHOLD else WINDOW_FULLY_OPEN_PERCENTAGE

            self.window_opening_percentage = max(0.01, min(WINDOW_FULLY_OPEN_PERCENTAGE, self.window_opening_percentage))
            new_sampling_freq = SAMPLING_FREQUENCY_F2_S
            self.too_hot_start_time = None # Reset timer
        else: # T > T2
            self.system_state = STATE_TOO_HOT
            self.window_opening_percentage = WINDOW_FULLY_OPEN_PERCENTAGE
            new_sampling_freq = SAMPLING_FREQUENCY_F2_S
            if self.too_hot_start_time is None:
                self.too_hot_start_time = time.time()
            elif time.time() - self.too_hot_start_time >= DT_ALARM_DURATION_S:
                self.system_state = STATE_ALARM
                logger.warning("System entered ALARM state!")
                # In ALARM, la finestra rimane completamente aperta come da TOO_HOT

        logger.debug(f"After eval: new self.window_opening_percentage = {self.window_opening_percentage}")
        logger.debug(f"Comparing: previous_window_opening ({previous_window_opening}) vs new_window_opening_percentage ({self.window_opening_percentage})")

        if previous_state != self.system_state:
             logger.info(f"System state changed: {previous_state} -> {self.system_state}")

        if self.mqtt_handler and new_sampling_freq:
            self.mqtt_handler.publish_sampling_frequency(new_sampling_freq)

        if self.serial_handler:
            # Invia il comando di posizione finestra se è cambiata
            if abs(previous_window_opening - self.window_opening_percentage) > 0.001:
                logger.info(f"AUTOMATIC MODE: Window percentage changed. Sending command for {self.window_opening_percentage*100:.0f}%")
                self.serial_handler.send_window_command(self.window_opening_percentage) # Es. SET_POS:X
            else:
                logger.info(f"AUTOMATIC MODE: Window percentage NOT significantly changed. Previous: {previous_window_opening*100:.0f}%, New: {self.window_opening_percentage*100:.0f}%. No command sent.")

            # Invia la modalità (anche se probabilmente è già AUTOMATIC qui,
            # ma se ci fosse un cambio di stato che implica un cambio di "sotto-modalità" futura,
            # potrebbe essere rilevante. Per ora, è ridondante se la modalità non cambia qui.)
            # self.serial_handler.send_system_mode(self.current_mode) # Probabilmente non serve qui se la modalità non è cambiata

            # In AUTOMATIC mode, l'Arduino non mostra la temperatura inviata dalla CU sull'LCD
            # quindi non è necessario inviare send_temperature_to_arduino.
            # L'LCD Arduino si aggiornerà con i dati che la sua FSM interna possiede
            # (modalità, e percentuale finestra comandata da SET_POS).

    # Chiamato da api_routes.py (richiesta dalla Dashboard) o da SerialHandler (pressione pulsante Arduino)
    def set_mode(self, mode):
        if mode in [MODE_AUTOMATIC, MODE_MANUAL]:
            if self.current_mode != mode:
                self.current_mode = mode
                logger.info(f"Mode changed to: {self.current_mode}")
                if self.current_mode == MODE_AUTOMATIC:
                    self._evaluate_automatic_mode() # Recalculate state and window
                else: # Switched to MANUAL
                    # Potrebbe essere utile inviare la frequenza F1 all'ESP
                    if self.mqtt_handler:
                        self.mqtt_handler.publish_sampling_frequency(SAMPLING_FREQUENCY_F1_S)
                if self.serial_handler:
                    self.serial_handler.send_system_mode(self.current_mode)
                    if self.current_mode == MODE_MANUAL and self.current_temperature is not None:
                        self.serial_handler.send_temperature_to_arduino(self.current_temperature)
            return True
        return False

    # Chiamato da api_routes.py (Dashboard) o SerialHandler (potenziometro Arduino) quando in MODE_MANUAL
    def set_manual_window_opening(self, percentage_str):
        try:
            percentage = float(percentage_str) / 100.0 # Assume Arduino invia 0-100
            percentage = max(WINDOW_CLOSED_PERCENTAGE, min(WINDOW_FULLY_OPEN_PERCENTAGE, percentage))
            if self.current_mode == MODE_MANUAL:
                if abs(self.window_opening_percentage - percentage) > 0.001:
                    self.window_opening_percentage = percentage
                    logger.info(f"Manual window opening set to: {self.window_opening_percentage*100:.0f}%")
                    if self.serial_handler:
                        # Invia il comando di posizione finestra
                        self.serial_handler.send_window_command(self.window_opening_percentage) # Es. SET_POS:X

                        # La modalità è già MANUAL (verificato all'inizio della funzione).
                        # Non serve inviare send_system_mode(MODE_MANUAL) di nuovo.

                        # Invia la temperatura corrente per l'LCD, dato che siamo in MANUALE
                        if self.current_temperature is not None:
                            self.serial_handler.send_temperature_to_arduino(self.current_temperature)
                return True
            else:
                logger.warning("Cannot set window opening: not in MANUAL mode.")
                return False
        except ValueError:
            logger.error(f"Invalid percentage value for manual window opening: {percentage_str}")
            return False

    # Se il sistema è in STATE_ALARM, lo riporta a STATE_NORMAL (o ricalcola), resetta il timer too_hot_start_time
    def handle_alarm_reset(self):
        if self.system_state == STATE_ALARM:
            self.system_state = STATE_NORMAL # O ricalcola in base alla temp corrente
            self.too_hot_start_time = None
            logger.info("ALARM state has been reset by operator.")
            # Rievaluta lo stato automatico per impostare finestra e frequenza corrette
            if self.current_mode == MODE_AUTOMATIC:
                 self._evaluate_automatic_mode()
            if self.serial_handler:
                # Notifica la modalità corrente (probabilmente AUTOMATIC se _evaluate_automatic_mode è stato chiamato)
                self.serial_handler.send_system_mode(self.current_mode)

                # Se la modalità dopo il reset è MANUALE (improbabile ma possibile), invia la temperatura
                if self.current_mode == MODE_MANUAL and self.current_temperature is not None:
                    self.serial_handler.send_temperature_to_arduino(self.current_temperature)

                # _evaluate_automatic_mode (se chiamato) si sarà occupato di inviare il SET_POS
                # Quindi, non è necessario inviare send_window_command qui di nuovo,
                # a meno che non ci sia un caso in cui _evaluate_automatic_mode non venga chiamato
                # ma la posizione finestra debba comunque essere comunicata.
                # In generale, dopo un reset di allarme, si ricalcola tutto e si inviano
                # i comandi necessari (posizione finestra, modo).
                # La chiamata a _evaluate_automatic_mode dovrebbe già aver aggiornato la posizione
                # della finestra e inviato il comando SET_POS.
            return True
        logger.info("Alarm reset requested, but system not in ALARM state.")
        return False

    # Prepara un dizionario con tutti i dati rilevanti da inviare alla Dashboard (temperature, medie, stato, modo, apertura finestra)
    def get_dashboard_data(self):
        return {
            "esp_status": self.esp_status,
            "current_temperature": self.current_temperature,
            "last_n_temperatures": list(self.last_n_temperatures),
            "average_temperature": self.avg_temp,
            "min_temperature": self.min_temp,
            "max_temperature": self.max_temp,
            "system_mode": self.current_mode,
            "system_state": self.system_state,
            "window_opening_percentage": self.window_opening_percentage * 100, # Invia come 0-100
            "alarm_active": self.system_state == STATE_ALARM
        }