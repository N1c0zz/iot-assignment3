# MQTT Configuration
MQTT_BROKER_ADDRESS = "localhost"
MQTT_BROKER_PORT = 1883
MQTT_TOPIC_TEMP_DATA = "assignment3/temperature"        # Topic da cui leggere i dati di temperatura dall'ESP
MQTT_TOPIC_TEMP_CONTROL = "assignment3/frequency"       # Topic su cui inviare comandi (es. freq) all'ESP
MQTT_TOPIC_ESP_STATUS = "assignment3/status"            # Topic per lo stato dell'ESP

# Serial Configuration
SERIAL_PORT = "COM4"                                      # Linux. Su Windows potrebbe essere "COM3", "COM4", etc.
SERIAL_BAUDRATE = 115200       # Deve corrispondere a quello dell'Arduino

# Control Logic Parameters
T1_THRESHOLD = 20  # Gradi Celsius - Soglia NORMAL -> HOT
T2_THRESHOLD = 27  # Gradi Celsius - Soglia HOT -> TOO_HOT
N_LAST_MEASUREMENTS = 10  # Numero di misure da tenere in memoria per statistiche
DT_ALARM_DURATION_S = 5  # Secondi in stato TOO_HOT prima di scattare ALARM

# Frequenze di campionamento (in secondi)
SAMPLING_FREQUENCY_F1_S = 60  # 1 campione al minuto (stato NORMAL - bassa frequenza)
SAMPLING_FREQUENCY_F2_S = 10  # 1 campione ogni 10 secondi (stati HOT/TOO_HOT - alta frequenza)

# Window Parameters
WINDOW_CLOSED_PERCENTAGE = 0.0
WINDOW_FULLY_OPEN_PERCENTAGE = 1.0 # 1.0 = 100%

# System Modes
MODE_AUTOMATIC = "AUTOMATIC"
MODE_MANUAL = "MANUAL"

# System States
STATE_NORMAL = "NORMAL"
STATE_HOT = "HOT"
STATE_TOO_HOT = "TOO_HOT"
STATE_ALARM = "ALARM"

# API Configuration
API_HOST = "0.0.0.0" # Ascolta su tutte le interfacce di rete
API_PORT = 5001