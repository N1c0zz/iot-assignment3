# api_routes.py
from flask import Blueprint, jsonify, request, current_app
import logging
from config.config import MODE_MANUAL, MODE_AUTOMATIC

logger = logging.getLogger(__name__)
api_bp = Blueprint('api', __name__, url_prefix='/api')

# Funzione per ottenere l'istanza di control_logic dall'app Flask
def get_control_logic():
    return current_app.control_logic_instance

@api_bp.route('/status', methods=['GET'])
def get_status():
    control_logic = get_control_logic()
    data = control_logic.get_dashboard_data()
    return jsonify(data)

@api_bp.route('/mode/manual', methods=['POST'])
def set_mode_manual():
    control_logic = get_control_logic()
    success = control_logic.set_mode(MODE_MANUAL)
    if success:
        return jsonify({"message": "Mode set to MANUAL"}), 200
    return jsonify({"message": "Failed to set mode"}), 400

@api_bp.route('/mode/automatic', methods=['POST'])
def set_mode_automatic():
    control_logic = get_control_logic()
    success = control_logic.set_mode(MODE_AUTOMATIC)
    if success:
        return jsonify({"message": "Mode set to AUTOMATIC"}), 200
    return jsonify({"message": "Failed to set mode"}), 400

@api_bp.route('/window/set', methods=['POST'])
def set_window_opening():
    control_logic = get_control_logic()
    if control_logic.current_mode != MODE_MANUAL:
        return jsonify({"message": "Cannot set window opening: system not in MANUAL mode"}), 403 # Forbidden

    data = request.get_json()
    if not data or 'percentage' not in data:
        return jsonify({"message": "Missing 'percentage' in request body"}), 400

    try:
        # Il frontend invia 0-100, set_manual_window_opening si aspetta una stringa 0-100
        percentage_str = str(float(data['percentage']))
        # IMPORTANTE: Specifica che questo comando viene dalla Dashboard
        success = control_logic.set_manual_window_opening(percentage_str, source="dashboard")
        if success:
            return jsonify({"message": f"Window opening set to {data['percentage']}%"}), 200
        else:
            return jsonify({"message": "Failed to set window opening"}), 400
    except ValueError:
        return jsonify({"message": "Invalid percentage value"}), 400


@api_bp.route('/alarm/reset', methods=['POST'])
def reset_alarm():
    control_logic = get_control_logic()
    success = control_logic.handle_alarm_reset()
    if success:
        return jsonify({"message": "Alarm reset successful"}), 200
    return jsonify({"message": "Failed to reset alarm (or system not in alarm state)"}), 400