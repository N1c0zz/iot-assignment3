"""
Control Logic Core Module for Smart Temperature Monitoring System.

This module implements the main control logic for the smart temperature monitoring
and window control system. It manages system states, automatic window control
based on temperature readings, and coordination between different system components.

The control logic implements a finite state machine with the following states:
- NORMAL: Temperature below T1_THRESHOLD, window closed, low sampling frequency
- HOT: Temperature between T1_THRESHOLD and T2_THRESHOLD, proportional window opening
- TOO_HOT: Temperature above T2_THRESHOLD, window fully open
- ALARM: System has been in TOO_HOT state for too long, requires operator intervention
"""

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
    """
    Core control logic for the smart temperature monitoring system.
    
    This class manages the system's finite state machine, processes temperature
    data, controls window opening based on temperature readings, and coordinates
    communication with external components (MQTT, Serial, Dashboard).
    """
    
    def __init__(self, mqtt_handler=None, serial_handler=None):
        """
        Initialize the control logic system.
        
        Args:
            mqtt_handler: Optional MQTT handler instance for ESP32 communication
            serial_handler: Optional serial handler instance for Arduino communication
        """
        # Communication handlers (injected dependencies)
        self.mqtt_handler = mqtt_handler
        self.serial_handler = serial_handler

        # System state variables
        self.current_mode = MODE_AUTOMATIC
        self.system_state = STATE_NORMAL
        self.esp_status = "UNKNOWN"
        self.esp_last_status_data = {}

        # Temperature tracking and statistics
        self.current_temperature = None
        self.last_n_temperatures = deque(maxlen=N_LAST_MEASUREMENTS)
        self.avg_temp = None
        self.min_temp = None
        self.max_temp = None

        # Window control
        self.window_opening_percentage = WINDOW_CLOSED_PERCENTAGE  # 0.0 to 1.0

        # Alarm system
        self.too_hot_start_time = None

        logger.info(f"ControlLogic initialized. Mode: {self.current_mode}, State: {self.system_state}")

    def update_esp_status(self, status, full_data_payload=None):
        """
        Update ESP32 sensor status information.
        
        Args:
            status: ESP32 status string (e.g., "online", "offline")
            full_data_payload: Optional complete status data payload
        """
        self.esp_status = status
        if full_data_payload:
            self.esp_last_status_data = full_data_payload
            
        logger.debug(f"ESP status updated: {status}")
        
        # Future enhancements could include:
        # - System notifications for offline sensors
        # - Fallback modes when sensor is unavailable
        # - Dashboard alerts for sensor issues

    def _initialize_state(self):
        """
        Initialize system state and send initial commands to external devices.
        
        Called during system startup when communication handlers are ready.
        Sets initial sampling frequency, system mode, and window position.
        """
        logger.info("Initializing system state...")
        
        # Set initial MQTT sampling frequency if connected
        if self.mqtt_handler and self.mqtt_handler.connected:
            self.mqtt_handler.publish_sampling_frequency(SAMPLING_FREQUENCY_F1_S)
            logger.info(f"Initial sampling frequency set: {SAMPLING_FREQUENCY_F1_S}s")
        
        # Initialize Arduino with current system mode and window position
        if self.serial_handler:
            self.serial_handler.send_system_mode(self.current_mode)
            
            # Send temperature if available and in manual mode
            if self.current_mode == MODE_MANUAL and self.current_temperature is not None:
                self.serial_handler.send_temperature_to_arduino(self.current_temperature)
            
            # Set initial window position
            self.serial_handler.send_window_command(self.window_opening_percentage)
            logger.info(f"Arduino initialized: mode={self.current_mode}, window={self.window_opening_percentage*100:.0f}%")

    def process_new_temperature(self, temp_value):
        """
        Process a new temperature reading from the ESP32 sensor.
        
        Args:
            temp_value: Temperature value in Celsius (float)
        """
        self.current_temperature = float(temp_value)
        self.last_n_temperatures.append(self.current_temperature)
        self._update_temperature_statistics()
        
        logger.info(f"New temperature: {self.current_temperature}°C (Mode: {self.current_mode})")

        if self.current_mode == MODE_AUTOMATIC:
            self._evaluate_automatic_mode()
        else:  # MANUAL mode
            # In manual mode, only send temperature to Arduino for LCD display
            if self.serial_handler and self.current_temperature is not None:
                self.serial_handler.send_temperature_to_arduino(self.current_temperature)

    def _update_temperature_statistics(self):
        """
        Update temperature statistics (average, min, max) from recent readings.
        """
        if self.last_n_temperatures:
            self.avg_temp = sum(self.last_n_temperatures) / len(self.last_n_temperatures)
            self.min_temp = min(self.last_n_temperatures)
            self.max_temp = max(self.last_n_temperatures)
            logger.debug(f"Temperature stats updated: avg={self.avg_temp:.1f}°C, min={self.min_temp:.1f}°C, max={self.max_temp:.1f}°C")
        else:
            self.avg_temp = None
            self.min_temp = None
            self.max_temp = None

    def _evaluate_automatic_mode(self):
        """
        Evaluate system state and control actions in automatic mode.
        
        Implements the main finite state machine logic:
        - Determines system state based on temperature thresholds
        - Controls window opening proportionally
        - Manages sampling frequency
        - Handles alarm conditions
        """
        if self.system_state == STATE_ALARM:
            logger.debug("System in ALARM state - waiting for operator intervention")
            return

        if self.current_temperature is None:
            logger.warning("Cannot evaluate automatic mode: current_temperature is None")
            return

        # Store previous state for change detection
        previous_state = self.system_state
        previous_window_opening = self.window_opening_percentage
        new_sampling_freq = None

        # State machine logic based on temperature thresholds
        if self.current_temperature < T1_THRESHOLD:
            self._transition_to_normal_state()
            new_sampling_freq = SAMPLING_FREQUENCY_F1_S
            
        elif T1_THRESHOLD <= self.current_temperature <= T2_THRESHOLD:
            self._transition_to_hot_state()
            new_sampling_freq = SAMPLING_FREQUENCY_F2_S
            
        else:  # Temperature > T2_THRESHOLD
            self._transition_to_too_hot_state()
            new_sampling_freq = SAMPLING_FREQUENCY_F2_S

        # Log state changes
        if previous_state != self.system_state:
            logger.info(f"System state changed: {previous_state} -> {self.system_state}")

        # Update MQTT sampling frequency if changed
        if self.mqtt_handler and new_sampling_freq:
            self.mqtt_handler.publish_sampling_frequency(new_sampling_freq)

        # Send window position command if significantly changed
        if self.serial_handler:
            window_change = abs(previous_window_opening - self.window_opening_percentage)
            if window_change > 0.001:  # Threshold to avoid unnecessary commands
                logger.info(f"AUTOMATIC: Window position changed to {self.window_opening_percentage*100:.0f}%")
                self.serial_handler.send_window_command(self.window_opening_percentage)

    def _transition_to_normal_state(self):
        """Handle transition to NORMAL state."""
        self.system_state = STATE_NORMAL
        self.window_opening_percentage = WINDOW_CLOSED_PERCENTAGE
        self.too_hot_start_time = None  # Reset alarm timer

    def _transition_to_hot_state(self):
        """Handle transition to HOT state with proportional window control."""
        self.system_state = STATE_HOT
        
        # Calculate proportional window opening: linear interpolation between T1 and T2
        # When T = T1: window opens to 1%
        # When T = T2: window opens to 100%
        if T2_THRESHOLD > T1_THRESHOLD:
            temp_range = T2_THRESHOLD - T1_THRESHOLD
            temp_offset = self.current_temperature - T1_THRESHOLD
            self.window_opening_percentage = (temp_offset / temp_range * 0.99) + 0.01
        else:
            # Edge case: if T1 == T2, use binary logic
            self.window_opening_percentage = 0.01 if self.current_temperature <= T1_THRESHOLD else WINDOW_FULLY_OPEN_PERCENTAGE

        # Constrain to valid range
        self.window_opening_percentage = max(0.01, min(WINDOW_FULLY_OPEN_PERCENTAGE, self.window_opening_percentage))
        self.too_hot_start_time = None  # Reset alarm timer

    def _transition_to_too_hot_state(self):
        """Handle transition to TOO_HOT state and alarm management."""
        self.system_state = STATE_TOO_HOT
        self.window_opening_percentage = WINDOW_FULLY_OPEN_PERCENTAGE
        
        # Start or continue alarm timer
        if self.too_hot_start_time is None:
            self.too_hot_start_time = time.time()
            logger.warning(f"TOO_HOT state entered - alarm timer started")
        elif time.time() - self.too_hot_start_time >= DT_ALARM_DURATION_S:
            self.system_state = STATE_ALARM
            logger.critical("ALARM state triggered - system requires operator intervention!")

    def set_mode(self, mode):
        """
        Set system operational mode (AUTOMATIC or MANUAL).
        
        Args:
            mode: Target operational mode (MODE_AUTOMATIC or MODE_MANUAL)
            
        Returns:
            bool: True if mode change successful, False otherwise
        """
        if mode not in [MODE_AUTOMATIC, MODE_MANUAL]:
            logger.error(f"Invalid mode requested: {mode}")
            return False

        if self.current_mode != mode:
            previous_mode = self.current_mode
            self.current_mode = mode
            logger.info(f"Mode changed: {previous_mode} -> {self.current_mode}")
            
            # Handle mode-specific initialization
            if self.current_mode == MODE_AUTOMATIC:
                self._on_enter_automatic_mode()
            else:  # MODE_MANUAL
                self._on_enter_manual_mode()
                
            # Update Arduino with new mode
            if self.serial_handler:
                self.serial_handler.send_system_mode(self.current_mode)
                
        return True

    def _on_enter_automatic_mode(self):
        """Actions to perform when entering AUTOMATIC mode."""
        logger.info("Entering AUTOMATIC mode")
        
        # Re-evaluate system state and apply automatic control
        self._evaluate_automatic_mode()
        
        # Set appropriate sampling frequency for ESP32
        if self.mqtt_handler:
            # Use high frequency if system is not in normal state
            freq = SAMPLING_FREQUENCY_F2_S if self.system_state != STATE_NORMAL else SAMPLING_FREQUENCY_F1_S
            self.mqtt_handler.publish_sampling_frequency(freq)

    def _on_enter_manual_mode(self):
        """Actions to perform when entering MANUAL mode."""
        logger.info("Entering MANUAL mode")
        
        # Switch to low frequency sampling in manual mode
        if self.mqtt_handler:
            self.mqtt_handler.publish_sampling_frequency(SAMPLING_FREQUENCY_F1_S)
            
        # Send current temperature to Arduino for LCD display
        if self.serial_handler and self.current_temperature is not None:
            self.serial_handler.send_temperature_to_arduino(self.current_temperature)

    def set_manual_window_opening(self, percentage_str, source="potentiometer"):
        """
        Set window opening percentage in manual mode.
        
        Args:
            percentage_str: Window opening percentage as string (0-100)
            source: Source of the command ("potentiometer" or "dashboard")
            
        Returns:
            bool: True if command successful, False otherwise
        """
        if self.current_mode != MODE_MANUAL:
            logger.warning("Cannot set manual window opening: system not in MANUAL mode")
            return False

        try:
            # Convert string percentage to float (0.0-1.0)
            percentage = max(0.0, min(100.0, float(percentage_str))) / 100.0
            
            # Only update if significantly different
            if abs(self.window_opening_percentage - percentage) > 0.001:
                self.window_opening_percentage = percentage
                logger.info(f"Manual window opening set to {percentage*100:.0f}% (source: {source})")
                
                # Send SET_POS command only if request comes from Dashboard
                # (potentiometer changes are already reflected physically)
                if source == "dashboard" and self.serial_handler:
                    self.serial_handler.send_window_command(self.window_opening_percentage)
                
                # Always send temperature update for LCD display
                if self.serial_handler and self.current_temperature is not None:
                    self.serial_handler.send_temperature_to_arduino(self.current_temperature)
                    
            return True
            
        except ValueError:
            logger.error(f"Invalid percentage value for manual window opening: {percentage_str}")
            return False

    def handle_alarm_reset(self):
        """
        Reset system from ALARM state back to normal operation.
        
        Returns:
            bool: True if alarm reset successful, False if system not in alarm
        """
        if self.system_state != STATE_ALARM:
            logger.info("Alarm reset requested, but system not in ALARM state")
            return False

        logger.info("ALARM state reset by operator")
        
        # Reset alarm timer and return to normal state
        self.system_state = STATE_NORMAL
        self.too_hot_start_time = None
        
        # Re-evaluate system state based on current temperature
        if self.current_mode == MODE_AUTOMATIC:
            self._evaluate_automatic_mode()
            
        # Update Arduino with current system state
        if self.serial_handler:
            self.serial_handler.send_system_mode(self.current_mode)
            
            # Send temperature if in manual mode
            if self.current_mode == MODE_MANUAL and self.current_temperature is not None:
                self.serial_handler.send_temperature_to_arduino(self.current_temperature)
                
        return True

    def get_dashboard_data(self):
        """
        Prepare system status data for dashboard API.
        
        Returns:
            dict: Complete system status including temperatures, mode, state, and window position
        """
        return {
            "esp_status": self.esp_status,
            "current_temperature": self.current_temperature,
            "last_n_temperatures": list(self.last_n_temperatures),
            "average_temperature": round(self.avg_temp, 2) if self.avg_temp is not None else None,
            "min_temperature": round(self.min_temp, 2) if self.min_temp is not None else None,
            "max_temperature": round(self.max_temp, 2) if self.max_temp is not None else None,
            "system_mode": self.current_mode,
            "system_state": self.system_state,
            "window_opening_percentage": round(self.window_opening_percentage * 100, 1),  # Convert to 0-100 range
            "alarm_active": self.system_state == STATE_ALARM
        }