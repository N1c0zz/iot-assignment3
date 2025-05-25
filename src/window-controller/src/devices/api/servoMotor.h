#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

/**
 * @class ServoMotor
 * @brief Interface for controlling a servo motor.
 *
 * Defines a contract for setting up a servo, moving it to a desired position
 * (expressed as a percentage), and querying its current set position.
 */
class ServoMotor {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ServoMotor() {}

    /**
     * @brief Initializes the servo motor.
     *        This typically involves attaching the servo to a specific pin
     *        and potentially setting an initial position.
     */
    virtual void setup() = 0;

    /**
     * @brief Sets the servo motor's position based on a percentage.
     * @param percentage The desired position, where 0% is typically the minimum angle
     *                   and 100% is the maximum angle defined for this servo's operational range.
     *                   The input will be constrained to 0-100.
     */
    virtual void setPositionPercentage(int percentage) = 0;

    /**
     * @brief Gets the current target position of the servo motor, as a percentage.
     * @return The current set position percentage (0-100).
     */
    virtual int getCurrentPercentage() const = 0;
};

#endif // SERVO_MOTOR_H