"""
Main Application Module for Control Unit Backend.

This module serves as the entry point for the control unit backend system.
It initializes all components (ControlLogic, MQTT handler, Serial handler),
sets up the Flask web server for the dashboard API, and manages the
application lifecycle including graceful shutdown.

The initialization sequence is critical:
1. Initialize ControlLogic core
2. Create and link MQTT and Serial handlers
3. Connect to Arduino (synchronous with timeout)
4. Connect to MQTT and start listening
5. Initialize system state
6. Start Flask web server
"""

from flask import Flask, send_from_directory
from flask_cors import CORS
import logging
import signal
import sys
import os

from config.config import API_HOST, API_PORT
from kernel.control_logic import ControlLogic
from communication.mqtt_handler import MqttHandler
from communication.serial_handler import SerialHandler
from api.api_routes import api_bp

# Directory Configuration
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_DIR = os.path.join(BASE_DIR, 'logs')
DASHBOARD_FRONTEND_DIR = os.path.join(BASE_DIR, '..', 'dashboard-frontend')

# Create logs directory if it doesn't exist
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR)

LOG_FILE_PATH = os.path.join(LOG_DIR, 'control_unit.log')

# Logging Configuration
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(LOG_FILE_PATH),
        logging.StreamHandler(sys.stdout)
    ]
)

# Disable werkzeug HTTP logs
logging.getLogger('werkzeug').setLevel(logging.ERROR)

logger = logging.getLogger(__name__)

# Global Component Instances
control_logic_instance = None
mqtt_handler_instance = None
serial_handler_instance = None
flask_app = None


def serve_index():
    """
    Serve the dashboard index.html file.
    
    Returns:
        Flask response with the dashboard HTML content
    """
    return send_from_directory(DASHBOARD_FRONTEND_DIR, 'index.html')


def serve_static(filename):
    """
    Serve static files (CSS, JS, images) for the dashboard.
    
    Args:
        filename: Name of the static file to serve
        
    Returns:
        Flask response with the requested static file
    """
    return send_from_directory(os.path.join(DASHBOARD_FRONTEND_DIR, 'static'), filename)


def signal_handler(sig, frame):
    """
    Handle system shutdown signals for graceful cleanup.
    
    This function is called when the system receives SIGINT (Ctrl+C) or
    SIGTERM signals. It ensures proper cleanup of MQTT and serial connections
    before terminating the application.
    
    Args:
        sig: Signal number
        frame: Current stack frame
    """
    logger.info("Shutdown signal received. Cleaning up...")
    
    # Stop MQTT communication
    if mqtt_handler_instance:
        mqtt_handler_instance.stop_listening_loop()
        
    # Stop serial communication
    if serial_handler_instance:
        serial_handler_instance.stop_listening()

    logger.info("Cleanup complete. Exiting.")
    sys.exit(0)


def initialize_control_logic():
    """
    Initialize the core control logic component.
    
    Returns:
        ControlLogic: Initialized control logic instance
    """
    logger.info("Initializing Control Logic...")
    return ControlLogic()


def initialize_communication_handlers(control_logic):
    """
    Initialize and configure MQTT and Serial communication handlers.
    
    Args:
        control_logic: ControlLogic instance to link with handlers
        
    Returns:
        tuple: (MqttHandler instance, SerialHandler instance)
    """
    logger.info("Initializing communication handlers...")
    
    # Create MQTT handler and establish bidirectional link
    mqtt_handler = MqttHandler(control_logic)
    control_logic.mqtt_handler = mqtt_handler
    
    # Create Serial handler and establish bidirectional link
    serial_handler = SerialHandler(control_logic)
    control_logic.serial_handler = serial_handler
    
    return mqtt_handler, serial_handler


def establish_connections(mqtt_handler, serial_handler):
    """
    Establish connections to external systems in the correct order.
    
    The connection sequence is important:
    1. Connect to Arduino first (synchronous with timeout)
    2. Connect to MQTT and start listening
    3. Initialize system state when both are ready
    
    Args:
        mqtt_handler: MqttHandler instance
        serial_handler: SerialHandler instance
        
    Returns:
        bool: True if all connections successful, False otherwise
    """
    # Connect to Arduino
    logger.info("Connecting to Arduino...")
    if not serial_handler.connect():
        logger.warning("Could not connect to Arduino. Window control will be unavailable.")
        logger.warning("System will continue but window commands will be lost.")
    else:
        logger.info("Arduino connection established successfully.")

    # Connect to MQTT broker
    logger.info("Connecting to MQTT broker...")
    mqtt_handler.connect()
    mqtt_handler.start_listening_loop()

    return True


def create_flask_app(control_logic):
    """
    Create and configure the Flask web application.
    
    Args:
        control_logic: ControlLogic instance to make available to routes
        
    Returns:
        Flask: Configured Flask application instance
    """
    logger.info("Creating Flask application...")
    
    app = Flask(
        __name__,
        static_folder=os.path.join(DASHBOARD_FRONTEND_DIR, 'static'),
        static_url_path='/static'
    )

    # Enable CORS for dashboard communication
    CORS(app)
    
    # Make control logic accessible to API routes
    app.control_logic_instance = control_logic
    
    # Register API blueprint
    app.register_blueprint(api_bp)
    
    # Add route for serving dashboard
    app.add_url_rule('/', view_func=serve_index)
    
    logger.info("Flask application configured successfully")
    return app


def main():
    """
    Main application entry point.
    
    Orchestrates the complete system initialization sequence:
    1. Initialize core control logic
    2. Set up communication handlers
    3. Establish external connections
    4. Initialize system state
    5. Start web server
    """
    global control_logic_instance, mqtt_handler_instance, serial_handler_instance, flask_app

    logger.info("=" * 60)
    logger.info("Starting Control Unit Backend System")
    logger.info("=" * 60)

    try:
        # Initialize core control logic
        control_logic_instance = initialize_control_logic()

        # Initialize communication handlers
        mqtt_handler_instance, serial_handler_instance = initialize_communication_handlers(control_logic_instance)

        # Establish connections to external systems
        establish_connections(mqtt_handler_instance, serial_handler_instance)

        # Initialize system state
        logger.info("Initializing system state...")
        control_logic_instance._initialize_state()

        # Create Flask application
        flask_app = create_flask_app(control_logic_instance)

        # Set up signal handlers for graceful shutdown
        signal.signal(signal.SIGINT, signal_handler)   # Ctrl+C
        signal.signal(signal.SIGTERM, signal_handler)  # kill command

        # Start the web server
        logger.info("=" * 60)
        logger.info(f"Control Unit Backend ready!")
        logger.info(f"Dashboard available at: http://{API_HOST}:{API_PORT}")
        logger.info("Press Ctrl+C to shutdown")
        logger.info("=" * 60)

        # Start Flask server
        flask_app.run(
            host=API_HOST, 
            port=API_PORT, 
            debug=False,
            use_reloader=False
        )

    except KeyboardInterrupt:
        logger.info("Keyboard interrupt received")
    except Exception as e:
        logger.error(f"Fatal error during startup: {e}", exc_info=True)
    finally:
        # Final cleanup
        logger.info("Performing final cleanup...")
        if mqtt_handler_instance and mqtt_handler_instance.connected:
            mqtt_handler_instance.stop_listening_loop()
        if serial_handler_instance and serial_handler_instance.is_running:
            serial_handler_instance.stop_listening()
        logger.info("Control Unit Backend shutdown complete")


if __name__ == '__main__':
    main()