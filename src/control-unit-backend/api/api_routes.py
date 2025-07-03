"""
API Routes Module for Control Unit Backend.

This module defines the REST API endpoints for the web dashboard to interact
with the control unit backend. It provides endpoints for system status retrieval,
mode changes, manual window control, and alarm management.
"""

from flask import Blueprint, jsonify, request, current_app
import logging
from config.config import MODE_MANUAL, MODE_AUTOMATIC, STATE_ALARM

logger = logging.getLogger(__name__)

# Create Flask blueprint for API routes
api_bp = Blueprint('api', __name__, url_prefix='/api')


def get_control_logic():
    """
    Get the control logic instance from the Flask application context.
    
    Returns:
        ControlLogic: The main control logic instance
    """
    return current_app.control_logic_instance


@api_bp.route('/status', methods=['GET'])
def get_status():
    """
    Get current system status for dashboard display.
    
    Returns comprehensive system information including:
    - Current and historical temperature data
    - System mode and state
    - Window opening percentage
    - ESP32 sensor status
    - Alarm status
    
    Returns:
        JSON response with complete system status
    """
    try:
        control_logic = get_control_logic()
        data = control_logic.get_dashboard_data()
        return jsonify(data), 200
    except Exception as e:
        logger.error(f"Error retrieving system status: {e}", exc_info=True)
        return jsonify({"error": "Failed to retrieve system status"}), 500


@api_bp.route('/mode/manual', methods=['POST'])
def set_mode_manual():
    """
    Switch system to MANUAL mode.
    
    In manual mode:
    - User controls window opening via potentiometer or dashboard
    - Temperature is displayed on Arduino LCD
    - Automatic temperature-based control is disabled
    
    Returns:
        JSON response indicating success or failure
    """
    try:
        control_logic = get_control_logic()

        # Check if system is in ALARM state
        if control_logic.system_state == STATE_ALARM:
            logger.info("Mode change to MANUAL blocked: system in ALARM state")
            return jsonify({"message": "Cannot change mode: system in ALARM state"}), 200

        success = control_logic.set_mode(MODE_MANUAL)
        
        if success:
            logger.info("System mode set to MANUAL via API")
            return jsonify({"message": "Mode set to MANUAL"}), 200
        else:
            logger.warning("Failed to set system mode to MANUAL")
            return jsonify({"message": "Failed to set mode"}), 400
            
    except Exception as e:
        logger.error(f"Error setting manual mode: {e}", exc_info=True)
        return jsonify({"error": "Internal server error"}), 500


@api_bp.route('/mode/automatic', methods=['POST'])
def set_mode_automatic():
    """
    Switch system to AUTOMATIC mode.
    
    In automatic mode:
    - System automatically controls window opening based on temperature
    - Window position calculated using proportional control
    - Sampling frequency adjusted based on system state
    
    Returns:
        JSON response indicating success or failure
    """
    try:
        control_logic = get_control_logic()

        # Check if system is in ALARM state
        if control_logic.system_state == STATE_ALARM:
            logger.info("Mode change to AUTOMATIC blocked: system in ALARM state")
            return jsonify({"message": "Cannot change mode: system in ALARM state"}), 200

        success = control_logic.set_mode(MODE_AUTOMATIC)
        
        if success:
            logger.info("System mode set to AUTOMATIC via API")
            return jsonify({"message": "Mode set to AUTOMATIC"}), 200
        else:
            logger.warning("Failed to set system mode to AUTOMATIC")
            return jsonify({"message": "Failed to set mode"}), 400
            
    except Exception as e:
        logger.error(f"Error setting automatic mode: {e}", exc_info=True)
        return jsonify({"error": "Internal server error"}), 500


@api_bp.route('/window/set', methods=['POST'])
def set_window_opening():
    """
    Set window opening percentage in manual mode.
    
    This endpoint allows the dashboard to directly control window position
    when the system is in MANUAL mode. The command is sent to the Arduino
    which will move the servo to the specified position.
    
    Expected JSON payload:
    {
        "percentage": <number between 0 and 100>
    }
    
    Returns:
        JSON response indicating success or failure
    """
    try:
        control_logic = get_control_logic()
        
        # Check if system is in manual mode
        if control_logic.current_mode != MODE_MANUAL:
            logger.warning("Window control attempted while not in MANUAL mode")
            return jsonify({"message": "Cannot set window opening: system not in MANUAL mode"}), 403

        # Validate request data
        data = request.get_json()
        if not data or 'percentage' not in data:
            logger.warning("Invalid window control request: missing percentage")
            return jsonify({"message": "Missing 'percentage' in request body"}), 400

        try:
            percentage = float(data['percentage'])
            if not (0 <= percentage <= 100):
                logger.warning(f"Invalid percentage value: {percentage}")
                return jsonify({"message": "Percentage must be between 0 and 100"}), 400
                
        except (ValueError, TypeError):
            logger.warning(f"Invalid percentage format: {data['percentage']}")
            return jsonify({"message": "Invalid percentage value"}), 400

        # Send command to control logic (specify dashboard as source)
        success = control_logic.set_manual_window_opening(str(percentage), source="dashboard")
        
        if success:
            logger.info(f"Window opening set to {percentage}% via dashboard")
            return jsonify({"message": f"Window opening set to {percentage}%"}), 200
        else:
            logger.error("Failed to set window opening")
            return jsonify({"message": "Failed to set window opening"}), 400

    except Exception as e:
        logger.error(f"Error setting window opening: {e}", exc_info=True)
        return jsonify({"error": "Internal server error"}), 500


@api_bp.route('/alarm/reset', methods=['POST'])
def reset_alarm():
    """
    Reset system alarm state.
    
    This endpoint allows operators to reset the system from ALARM state
    back to normal operation. The alarm is triggered when the system
    remains in TOO_HOT state for longer than the configured duration.
    
    Returns:
        JSON response indicating success or failure
    """
    try:
        control_logic = get_control_logic()
        success = control_logic.handle_alarm_reset()
        
        if success:
            logger.info("System alarm reset via dashboard")
            return jsonify({"message": "Alarm reset successful"}), 200
        else:
            logger.info("Alarm reset requested but system not in alarm state")
            return jsonify({"message": "Failed to reset alarm (system not in alarm state)"}), 400
            
    except Exception as e:
        logger.error(f"Error resetting alarm: {e}", exc_info=True)
        return jsonify({"error": "Internal server error"}), 500


# Error handlers for the API blueprint
@api_bp.errorhandler(404)
def not_found(error):
    """Handle 404 errors for API routes."""
    return jsonify({"error": "API endpoint not found"}), 404


@api_bp.errorhandler(500)
def internal_error(error):
    """Handle 500 errors for API routes."""
    logger.error(f"Internal server error in API: {error}")
    return jsonify({"error": "Internal server error"}), 500