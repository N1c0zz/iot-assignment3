from flask import Flask, send_from_directory
from flask_cors import CORS
import logging
import signal
import sys
import time
import os

from config.config import (
    API_HOST, API_PORT,
    MODE_AUTOMATIC, # Aggiunta
    STATE_TOO_HOT,  # Aggiunta se usata nel test dell'allarme
    T2_THRESHOLD,   # Aggiunta se usata nel test dell'allarme
    DT_ALARM_DURATION_S # Aggiunta se usata nel test dell'allarme
)
from kernel.control_logic import ControlLogic
from communication.mqtt_handler import MqttHandler
from communication.serial_handler import SerialHandler
from api.api_routes import api_bp

# --- Base Directory ---
# Ottiene il path assoluto della directory in cui si trova app.py
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_DIR = os.path.join(BASE_DIR, 'logs') # Path alla directory logs
DASHBOARD_FRONTEND_DIR = os.path.join(BASE_DIR, '..', 'dashboard-frontend')

# Crea la directory logs se non esiste
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR)

LOG_FILE_PATH = os.path.join(LOG_DIR, 'control_unit.log') # Path completo al file di log

# --- Logging Configuration ---
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(LOG_FILE_PATH), # Usa il path assoluto
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger(__name__)

# --- Global instances ---
# Questi verranno inizializzati in main
control_logic_instance = None
mqtt_handler_instance = None
serial_handler_instance = None
flask_app = None

# Serve index.html
def serve_index():
    return send_from_directory(DASHBOARD_FRONTEND_DIR, 'index.html')

# Serve i file statici (CSS, JS, immagini...)
def serve_static(filename):
    return send_from_directory(os.path.join(DASHBOARD_FRONTEND_DIR, 'static'), filename)

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

    # 4. PRIMA: Connetti Arduino (operazione sincrona con timeout)
    logger.info("Attempting to connect to Arduino first...")
    if not serial_handler_instance.connect():
        logger.warning("Could not connect to Arduino. Window control will be unavailable.")
        logger.warning("System will continue but window commands will be lost.")
    else:
        logger.info("Arduino connection established successfully.")

    # 5. DOPO: Connetti MQTT e inizia il listening (ora Arduino è pronto)
    logger.info("Arduino ready, now connecting to MQTT...")
    mqtt_handler_instance.connect()
    mqtt_handler_instance.start_listening_loop()
    
    # 6. INFINE: Inizializza lo stato del sistema
    # Ora sia MQTT che Serial sono pronti, quindi nessun comando andrà perso
    logger.info("Both Arduino and MQTT ready, initializing system state...")
    control_logic_instance._initialize_state()

    # 7. Inizializza Flask App
    app = Flask(
        __name__,
        static_folder=os.path.join(DASHBOARD_FRONTEND_DIR, 'static'),
        static_url_path='/static')

    CORS(app)
    app.control_logic_instance = control_logic_instance # Rendi control_logic accessibile alle route
    # Registra il blueprint per le API
    app.register_blueprint(api_bp)

    # Aggiungi la route per servire index.html
    app.add_url_rule('/', view_func=serve_index)

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