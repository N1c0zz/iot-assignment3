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
T1_THRESHOLD = 20.0  # Gradi Celsius
T2_THRESHOLD = 25.0  # Gradi Celsius
N_LAST_MEASUREMENTS = 20
DT_ALARM_DURATION_S = 300  # Secondi (5 minuti) per lo stato TOO_HOT prima dell'ALARM
SAMPLING_FREQUENCY_F1_S = 60 # Secondi (1 campione al minuto)
SAMPLING_FREQUENCY_F2_S = 10 # Secondi (1 campione ogni 10 secondi)

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
API_PORT = 5000