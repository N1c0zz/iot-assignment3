/**
 * Dashboard Application Class
 * Manages the frontend interface for the Smart Temperature Monitoring System
 */
class TemperatureDashboard {
    constructor() {
        // Configuration constants
        this.API_BASE_URL = 'http://localhost:5001/api';
        this.POLLING_INTERVAL_MS = 3000; // 3 seconds
        this.INTERACTION_TIMEOUT = 2000; // 2 seconds for user interaction timeout
        
        // Application state
        this.currentSystemData = null;
        this.temperatureChart = null;
        this.isUserInteracting = false;
        this.lastUserInteraction = 0;
        
        // DOM elements cache
        this.elements = {};
        
        // Bind methods to maintain context
        this.fetchData = this.fetchData.bind(this);
        this.sendCommand = this.sendCommand.bind(this);
    }

    /**
     * Initialize the dashboard application
     */
    init() {
        this.cacheElements();
        this.initializeChart();
        this.attachEventListeners();
        this.startDataPolling();
        console.log('Temperature Dashboard initialized successfully');
    }

    /**
     * Cache DOM elements for better performance
     */
    cacheElements() {
        // Status display elements
        this.elements.currentTemp = document.getElementById('current-temp');
        this.elements.avgTemp = document.getElementById('avg-temp');
        this.elements.minTemp = document.getElementById('min-temp');
        this.elements.maxTemp = document.getElementById('max-temp');
        this.elements.systemState = document.getElementById('system-state');
        this.elements.windowOpening = document.getElementById('window-opening');
        this.elements.systemMode = document.getElementById('system-mode');
        this.elements.espStatus = document.getElementById('esp-status');

        // Control elements
        this.elements.btnSetAuto = document.getElementById('btn-set-auto');
        this.elements.btnSetManual = document.getElementById('btn-set-manual');
        this.elements.btnResetAlarm = document.getElementById('btn-reset-alarm');
        this.elements.btnSetWindowManual = document.getElementById('btn-set-window-manual');
        
        // Manual control elements
        this.elements.manualControlsContainer = document.getElementById('manual-window-controls');
        this.elements.windowSlider = document.getElementById('window-slider');
        this.elements.sliderValue = document.getElementById('slider-value');
        
        // Chart element
        this.elements.chartCanvas = document.getElementById('temperatureChart');
    }

    /**
     * Initialize the temperature chart using Chart.js
     */
    initializeChart() {
        const ctx = this.elements.chartCanvas.getContext('2d');
        
        this.temperatureChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Temperature (°C)',
                    data: [],
                    borderColor: 'rgb(75, 192, 192)',
                    backgroundColor: 'rgba(75, 192, 192, 0.1)',
                    tension: 0.1,
                    fill: false,
                    pointRadius: 4,
                    pointHoverRadius: 6
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: false,
                        title: {
                            display: true,
                            text: 'Temperature (°C)'
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Measurement Sequence (most recent on right)'
                        }
                    }
                },
                plugins: {
                    legend: {
                        display: true,
                        position: 'top'
                    },
                    tooltip: {
                        mode: 'index',
                        intersect: false
                    }
                }
            }
        });
    }

    /**
     * Attach event listeners to UI elements
     */
    attachEventListeners() {
        // Mode control buttons
        this.elements.btnSetAuto.addEventListener('click', () => {
            this.sendCommand('/mode/automatic');
        });
        
        this.elements.btnSetManual.addEventListener('click', () => {
            this.sendCommand('/mode/manual');
        });
        
        this.elements.btnResetAlarm.addEventListener('click', () => {
            this.sendCommand('/alarm/reset');
        });

        // Manual window control
        this.elements.btnSetWindowManual.addEventListener('click', () => {
            this.handleManualWindowSet();
        });

        // Slider interaction tracking
        this.attachSliderEventListeners();
    }

    /**
     * Attach event listeners for slider interaction tracking
     */
    attachSliderEventListeners() {
        const slider = this.elements.windowSlider;

        // Mouse events
        slider.addEventListener('mousedown', () => {
            this.isUserInteracting = true;
            console.log('User started interacting with slider (mouse)');
        });

        slider.addEventListener('mouseup', () => {
            this.isUserInteracting = false;
            console.log('User finished interacting with slider (mouse)');
        });

        // Touch events for mobile devices
        slider.addEventListener('touchstart', () => {
            this.isUserInteracting = true;
            console.log('User started interacting with slider (touch)');
        });

        slider.addEventListener('touchend', () => {
            this.isUserInteracting = false;
            console.log('User finished interacting with slider (touch)');
        });

        // Input change event
        slider.addEventListener('input', () => {
            this.elements.sliderValue.textContent = slider.value;
            this.lastUserInteraction = Date.now();
            console.log(`Slider changed by user: ${slider.value}%`);
        });
    }

    /**
     * Handle manual window position setting
     */
    handleManualWindowSet() {
        const percentage = parseInt(this.elements.windowSlider.value, 10);
        
        if (this.currentSystemData && this.currentSystemData.system_mode === 'MANUAL') {
            this.sendCommand('/window/set', { percentage: percentage });
        } else {
            alert('System must be in MANUAL mode to set window manually.');
        }
    }

    /**
     * Start the data polling loop
     */
    startDataPolling() {
        // Fetch initial data
        this.fetchData();
        
        // Set up periodic polling
        setInterval(this.fetchData, this.POLLING_INTERVAL_MS);
    }

    /**
     * Fetch system status data from the API
     */
    async fetchData() {
        try {
            const response = await fetch(`${this.API_BASE_URL}/status`);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status} ${response.statusText}`);
            }
            
            const data = await response.json();
            this.updateUI(data);
            
        } catch (error) {
            console.error('Failed to fetch data from Control Unit:', error);
            this.handleFetchError(error);
        }
    }

    /**
     * Handle fetch errors by updating UI appropriately
     */
    handleFetchError(error) {
        // Update system state to show error
        this.elements.systemState.textContent = 'Connection Error';
        this.elements.systemState.className = 'state-alarm';
        
        // You could add more sophisticated error handling here
        console.error('Dashboard connection error:', error);
    }

    /**
     * Update the user interface with new data
     * @param {Object} data - System status data from API
     */
    updateUI(data) {
        // Store current data
        this.currentSystemData = data;

        // Update temperature displays
        this.updateTemperatureDisplays(data);
        
        // Update system status displays
        this.updateSystemStatus(data);
        
        // Update chart
        this.updateTemperatureChart(data);
        
        // Update control panel visibility and state
        this.updateControlPanel(data);
    }

    /**
     * Update temperature-related displays
     */
    updateTemperatureDisplays(data) {
        this.elements.currentTemp.textContent = 
            data.current_temperature !== null ? data.current_temperature.toFixed(1) : '-';
        
        this.elements.avgTemp.textContent = 
            data.average_temperature !== null ? data.average_temperature.toFixed(1) : '-';
        
        this.elements.minTemp.textContent = 
            data.min_temperature !== null ? data.min_temperature.toFixed(1) : '-';
        
        this.elements.maxTemp.textContent = 
            data.max_temperature !== null ? data.max_temperature.toFixed(1) : '-';
    }

    /**
     * Update system status displays
     */
    updateSystemStatus(data) {
        // System state with appropriate CSS class
        this.elements.systemState.textContent = data.system_state || '-';
        this.elements.systemState.className = '';
        if (data.system_state) {
            const stateClass = `state-${data.system_state.toLowerCase().replace('_', '-')}`;
            this.elements.systemState.classList.add(stateClass);
        }

        // Window opening percentage
        this.elements.windowOpening.textContent = 
            data.window_opening_percentage !== null ? data.window_opening_percentage.toFixed(0) : '-';
        
        // System mode
        this.elements.systemMode.textContent = data.system_mode || '-';
        
        // ESP status
        this.elements.espStatus.textContent = data.esp_status || 'UNKNOWN';
    }

    /**
     * Update the temperature chart
     */
    updateTemperatureChart(data) {
        if (this.temperatureChart && data.last_n_temperatures) {
            // Create simple sequential labels
            const labels = data.last_n_temperatures.map((_, index) => index + 1);
            
            // Update chart data
            this.temperatureChart.data.labels = labels;
            this.temperatureChart.data.datasets[0].data = data.last_n_temperatures;
            this.temperatureChart.update('none'); // 'none' for better performance
        }
    }

    /**
     * Update control panel based on system state
     */
    updateControlPanel(data) {
        // Check if system is in alarm state
        const isAlarmState = (data.system_state === 'ALARM');
        
        // Update mode buttons state
        this.elements.btnSetAuto.disabled = (data.system_mode === 'AUTOMATIC') || isAlarmState;
        this.elements.btnSetManual.disabled = (data.system_mode === 'MANUAL') || isAlarmState;

        // Show/hide manual controls
        if (data.system_mode === 'MANUAL') {
            this.elements.manualControlsContainer.classList.remove('hidden');
            this.updateManualSlider(data);
        } else {
            this.elements.manualControlsContainer.classList.add('hidden');
        }

        // Show/hide alarm reset button
        if (data.system_state === 'ALARM') {
            this.elements.btnResetAlarm.classList.remove('hidden');
        } else {
            this.elements.btnResetAlarm.classList.add('hidden');
        }
    }

    /**
     * Update manual slider with protection against user interaction conflicts
     */
    updateManualSlider(data) {
        if (data.window_opening_percentage !== null) {
            const newPercentage = data.window_opening_percentage.toFixed(0);
            const currentSliderValue = parseInt(this.elements.windowSlider.value, 10);
            const timeSinceLastInteraction = Date.now() - this.lastUserInteraction;
            
            // Only update slider if user is not actively interacting
            const shouldUpdateSlider = !this.isUserInteracting && 
                                      timeSinceLastInteraction > this.INTERACTION_TIMEOUT &&
                                      document.activeElement !== this.elements.windowSlider &&
                                      currentSliderValue !== parseInt(newPercentage, 10);
            
            if (shouldUpdateSlider) {
                this.elements.windowSlider.value = newPercentage;
                this.elements.sliderValue.textContent = newPercentage;
                console.log(`Slider automatically updated to: ${newPercentage}%`);
            }
        }
    }

    /**
     * Send command to the Control Unit API
     * @param {string} endpoint - API endpoint (e.g., '/mode/manual')
     * @param {Object} body - Request body data (optional)
     */
    async sendCommand(endpoint, body = null) {
        try {
            const options = {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                }
            };
            
            if (body) {
                options.body = JSON.stringify(body);
            }
            
            const response = await fetch(`${this.API_BASE_URL}${endpoint}`, options);
            
            if (!response.ok) {
                const errorData = await response.json().catch(() => ({ 
                    message: 'Unknown error' 
                }));
                throw new Error(`HTTP error! status: ${response.status} - ${errorData.message || response.statusText}`);
            }
            
            const result = await response.json();
            console.log(`Command ${endpoint} successful:`, result.message);
            
            // Immediately fetch updated data
            setTimeout(() => this.fetchData(), 100);
            
        } catch (error) {
            console.error(`Error sending command ${endpoint}:`, error);
            alert(`Error: ${error.message}`);
        }
    }
}

// Initialize dashboard when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    const dashboard = new TemperatureDashboard();
    dashboard.init();
});