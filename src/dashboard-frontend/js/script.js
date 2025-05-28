// js/script.js
document.addEventListener('DOMContentLoaded', () => {
    // ----- CONFIGURAZIONE -----
    const API_BASE_URL = 'http://localhost:5000/api'; // Cambia se usi ngrok o altro
    const POLLING_INTERVAL_MS = 3000; // Intervallo per aggiornare i dati (es. 3 secondi)

    // ----- ELEMENTI UI -----
    // Display Status
    const currentTempEl = document.getElementById('current-temp');
    const avgTempEl = document.getElementById('avg-temp');
    const minTempEl = document.getElementById('min-temp');
    const maxTempEl = document.getElementById('max-temp');
    const systemStateEl = document.getElementById('system-state');
    const windowOpeningEl = document.getElementById('window-opening');
    const systemModeEl = document.getElementById('system-mode');
    const espStatusEl = document.getElementById('esp-status');

    // Controlli Modalità
    const btnSetAuto = document.getElementById('btn-set-auto');
    const btnSetManual = document.getElementById('btn-set-manual');

    // Controlli Finestra Manuale
    const manualControlsContainer = document.getElementById('manual-window-controls');
    const windowSlider = document.getElementById('window-slider');
    const sliderValueEl = document.getElementById('slider-value');
    const btnSetWindowManual = document.getElementById('btn-set-window-manual');

    // Controlli Allarme
    const btnResetAlarm = document.getElementById('btn-reset-alarm');

    // Grafico Temperatura
    const chartCtx = document.getElementById('temperatureChart').getContext('2d');
    let temperatureChart; // Verrà inizializzato dopo

    // ----- STATO APPLICAZIONE (SE NECESSARIO) -----
    let currentSystemData = null; // Per tenere traccia dei dati più recenti

    // ----- FUNZIONI -----

    /**
     * Inizializza il grafico della temperatura.
     * @param {Array<Number>} initialData - Array di temperature iniziali.
     * @param {Array<String>} initialLabels - Array di etichette per i dati.
     */
    function initializeChart(initialData = [], initialLabels = []) {
        temperatureChart = new Chart(chartCtx, {
            type: 'line',
            data: {
                labels: initialLabels,
                datasets: [{
                    label: 'Temperature (°C)',
                    data: initialData,
                    borderColor: 'rgb(75, 192, 192)',
                    tension: 0.1,
                    fill: false
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false, // Permette al grafico di adattarsi meglio
                scales: {
                    y: {
                        beginAtZero: false, // Non iniziare per forza da zero per le temperature
                        title: {
                            display: true,
                            text: 'Temperature (°C)'
                        }
                    },
                    x: {
                         title: {
                            display: true,
                            text: 'Time (most recent on right)'
                        }
                    }
                }
            }
        });
    }

    /**
     * Aggiorna l'interfaccia utente con i dati ricevuti dalla Control Unit.
     * @param {Object} data - L'oggetto dati dall'API /api/status.
     */
    function updateUI(data) {
        currentSystemData = data; // Salva i dati più recenti

        currentTempEl.textContent = data.current_temperature !== null ? data.current_temperature.toFixed(1) : '-';
        avgTempEl.textContent = data.average_temperature !== null ? data.average_temperature.toFixed(1) : '-';
        minTempEl.textContent = data.min_temperature !== null ? data.min_temperature.toFixed(1) : '-';
        maxTempEl.textContent = data.max_temperature !== null ? data.max_temperature.toFixed(1) : '-';
        
        systemStateEl.textContent = data.system_state || '-';
        systemStateEl.className = ''; // Resetta classi
        if (data.system_state) {
            systemStateEl.classList.add(`state-${data.system_state.toLowerCase().replace('_', '-')}`);
        }

        windowOpeningEl.textContent = data.window_opening_percentage !== null ? data.window_opening_percentage.toFixed(0) : '-';
        systemModeEl.textContent = data.system_mode || '-';
        espStatusEl.textContent = data.esp_status || 'UNKNOWN';

        // Aggiorna Grafico
        if (temperatureChart && data.last_n_temperatures) {
            // Crea etichette semplici (es. da 1 a N) o potresti usare timestamp se disponibili
            const labels = data.last_n_temperatures.map((_, index) => index + 1);
            temperatureChart.data.labels = labels;
            temperatureChart.data.datasets[0].data = data.last_n_temperatures;
            temperatureChart.update();
        }

        // Gestisci visibilità controlli manuali e pulsante allarme
        if (data.system_mode === 'MANUAL') {
            manualControlsContainer.classList.remove('hidden');
            if (data.window_opening_percentage !== null) {
                windowSlider.value = data.window_opening_percentage.toFixed(0);
                sliderValueEl.textContent = data.window_opening_percentage.toFixed(0);
            }
        } else {
            manualControlsContainer.classList.add('hidden');
        }

        if (data.system_state === 'ALARM') {
            btnResetAlarm.classList.remove('hidden');
        } else {
            btnResetAlarm.classList.add('hidden');
        }

        // Abilita/disabilita pulsanti di modo
        btnSetAuto.disabled = data.system_mode === 'AUTOMATIC';
        btnSetManual.disabled = data.system_mode === 'MANUAL';
    }

    /**
     * Recupera i dati di stato dalla Control Unit.
     */
    async function fetchData() {
        try {
            const response = await fetch(`${API_BASE_URL}/status`);
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status} ${response.statusText}`);
            }
            const data = await response.json();
            updateUI(data);
        } catch (error) {
            console.error("Could not fetch data from Control Unit:", error);
            // Potresti mostrare un messaggio di errore sull'UI qui
            // Esempio: systemStateEl.textContent = "Error connecting"; systemStateEl.className = 'state-alarm';
        }
    }

    /**
     * Invia un comando POST alla Control Unit.
     * @param {String} endpoint - L'endpoint API (es. /mode/manual).
     * @param {Object} body - L'oggetto da inviare come JSON nel corpo della richiesta.
     */
    async function sendCommand(endpoint, body = null) {
        try {
            const options = {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
            };
            if (body) {
                options.body = JSON.stringify(body);
            }
            const response = await fetch(`${API_BASE_URL}${endpoint}`, options);
            if (!response.ok) {
                const errorData = await response.json().catch(() => ({ message: "Unknown error" }));
                throw new Error(`HTTP error! status: ${response.status} - ${errorData.message || response.statusText}`);
            }
            const result = await response.json();
            console.log(`Command ${endpoint} successful:`, result.message);
            fetchData(); // Aggiorna UI dopo il comando
        } catch (error) {
            console.error(`Error sending command ${endpoint}:`, error);
            alert(`Error: ${error.message}`); // Mostra errore all'utente
        }
    }

    // ----- EVENT LISTENERS -----
    btnSetAuto.addEventListener('click', () => sendCommand('/mode/automatic'));
    btnSetManual.addEventListener('click', () => sendCommand('/mode/manual'));
    btnResetAlarm.addEventListener('click', () => sendCommand('/alarm/reset'));

    windowSlider.addEventListener('input', () => {
        sliderValueEl.textContent = windowSlider.value;
    });

    btnSetWindowManual.addEventListener('click', () => {
        const percentage = parseInt(windowSlider.value, 10);
        if (currentSystemData && currentSystemData.system_mode === 'MANUAL') {
            sendCommand('/window/set', { percentage: percentage });
        } else {
            alert("System must be in MANUAL mode to set window manually.");
        }
    });

    // ----- INIZIALIZZAZIONE -----
    initializeChart(); // Inizializza grafico vuoto
    fetchData(); // Recupera i dati iniziali
    setInterval(fetchData, POLLING_INTERVAL_MS); // Imposta polling per aggiornamenti
});