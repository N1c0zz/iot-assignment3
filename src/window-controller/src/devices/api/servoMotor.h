#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

/**
 * @class ServoMotor
 * @brief Abstract interface for servo motor control
 * 
 * This interface defines the contract for controlling a servo motor used
 * in window positioning applications. It provides a percentage-based
 * abstraction layer over the underlying angular servo control.
 */
class ServoMotor {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~ServoMotor() = default;

    /**
     * @brief Initialize the servo motor hardware
     * 
     * This method should:
     * - Configure the servo motor pin
     * - Attach the servo to the PWM pin
     * - Set initial position (typically 0% - closed)
     * - Perform any hardware-specific initialization
     */
    virtual void setup() = 0;

    /**
     * @brief Set servo position based on percentage
     * 
     * Commands the servo motor to move to the specified position.
     * The percentage is mapped to the servo's configured angular range.
     * 
     * @param percentage Target position (0% = fully closed, 100% = fully open)
     */
    virtual void setPositionPercentage(int percentage) = 0;

    /**
     * @brief Get current servo target position
     * 
     * Returns the last commanded position percentage, not necessarily
     * the actual physical position (which may still be moving).
     * 
     * @return Current target position as percentage (0-100)
     */
    virtual int getCurrentPercentage() const = 0;
};

#endif // SERVO_MOTOR_H