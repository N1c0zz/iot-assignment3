from flask import Flask
import logging
import signal
import sys
import time

from config.config import API_HOST, API_PORT
from kernel.control_logic import ControlLogic
from communication.mqtt_handler import MqttHandler
from communication.serial_handler import SerialHandler
from communication.api_routes import api_bp

# --- Logging Configuration ---
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("src/control-unit-backend/logs/control_unit.log"), # Log su file
        logging.StreamHandler(sys.stdout)       # Log su console
    ]
)
logger = logging.getLogger(__name__)

# --- Global instances ---
# Questi verranno inizializzati in main
control_logic_instance = None
mqtt_handler_instance = None
serial_handler_instance = None
flask_app = None

# Funzione eseguita quando il sistema operativo manda un segnale specifico al processo Python
# in questo caso pensata per interrompere la connessione MQTT e la connessione sulla Serial
# Line in seguito ad un segnale di arresto (per avere una chiusura più pulita)
def signal_handler(sig, frame):
    logger.info("Shutdown signal received. Cleaning up...")
    if mqtt_handler_instance:
        mqtt_handler_instance.stop_listening_loop()
    if serial_handler_instance:
        serial_handler_instance.stop_listening()

    logger.info("Cleanup complete. Exiting.")
    sys.exit(0)

def main():
    global control_logic_instance, mqtt_handler_instance, serial_handler_instance, flask_app

    logger.info("Starting Control Unit Backend...")

    # 1. Inizializza ControlLogic (senza handler per ora, verranno settati dopo)
    control_logic_instance = ControlLogic()

    # 2. Inizializza MQTT Handler e passa l'istanza di ControlLogic (link bidirezionale)
    mqtt_handler_instance = MqttHandler(control_logic_instance)
    control_logic_instance.mqtt_handler = mqtt_handler_instance

    # 3. Inizializza Serial Handler e passa l'istanza di ControlLogic (link bidirezionale)
    serial_handler_instance = SerialHandler(control_logic_instance)
    control_logic_instance.serial_handler = serial_handler_instance

    # 4. Connetti MQTT e Seriale
    mqtt_handler_instance.connect()
    mqtt_handler_instance.start_listening_loop()

    # Tenta la connessione seriale. Se fallisce, il sistema può comunque partire
    # ma la funzionalità della finestra sarà limitata.
    if not serial_handler_instance.connect():
        logger.warning("Could not connect to Arduino. Window control might be unavailable.")
    # else:
        # Se la connessione seriale ha successo, inizializziamo lo stato dopo che tutto è connesso
        # Questo è stato spostato nel costruttore di ControlLogic, che ora riceve gli handler
        # e può chiamare i loro metodi dopo la sua inizializzazione.
        # Però, una chiamata esplicita qui dopo che TUTTO è connesso potrebbe essere più robusta.
        # control_logic_instance._initialize_state() # Per inviare stato iniziale a ESP/Arduino

    # Re-initialize state which might send initial commands via MQTT/Serial
    # Ora che gli handler sono settati in control_logic_instance.
    control_logic_instance._initialize_state()


    # 5. Inizializza Flask App
    app = Flask(__name__)
    app.control_logic_instance = control_logic_instance # Rendi control_logic accessibile alle route
    app.register_blueprint(api_bp)
    flask_app = app # Salva per signal_handler

    # Gestione spegnimento pulito
    signal.signal(signal.SIGINT, signal_handler)  # CTRL+C
    signal.signal(signal.SIGTERM, signal_handler) # `kill` command

    logger.info(f"Flask API server starting on http://{API_HOST}:{API_PORT}")
    try:
        # debug=False e use_reloader=False sono importanti per la produzione e per evitare problemi con i thread.
        # Per lo sviluppo, debug=True è comodo ma può causare l'esecuzione doppia di parti di codice.
        app.run(host=API_HOST, port=API_PORT, debug=False, use_reloader=False)
    except Exception as e:
        logger.error(f"Flask app run failed: {e}")
    finally:
        # Questo blocco finally potrebbe non essere raggiunto se app.run() è bloccante e SIGINT lo interrompe prima.
        # Ecco perché signal_handler è il meccanismo primario di cleanup.
        logger.info("Flask app has stopped.")
        # Assicurati che il cleanup venga chiamato anche se Flask si ferma per altri motivi
        if mqtt_handler_instance and mqtt_handler_instance.connected:
             mqtt_handler_instance.stop_listening_loop()
        if serial_handler_instance and serial_handler_instance.is_running:
             serial_handler_instance.stop_listening()


if __name__ == '__main__':
    main()