#ifndef SERVO_MOTOR_IMPL_H
#define SERVO_MOTOR_IMPL_H

#include "ServoMotor.h"
#include <Arduino.h>
#include <Servo.h>

/**
 * @class ServoMotorImpl
 * @brief Arduino Servo library implementation of ServoMotor interface
 * 
 * This class provides concrete servo motor control using the standard
 * Arduino Servo library. It maps percentage-based position commands
 * to angular servo positions within a configurable range.
 */
class ServoMotorImpl : public ServoMotor {
public:
    /**
     * @brief Construct servo motor controller
     * 
     * @param pin Arduino PWM pin connected to servo signal line
     * @param minAngle Servo angle (degrees) for 0% position (typically 0°)
     * @param maxAngle Servo angle (degrees) for 100% position (typically 90° or 180°)
     */
    ServoMotorImpl(int pin, int minAngle, int maxAngle);

    /**
     * @brief Default destructor
     */
    virtual ~ServoMotorImpl() = default;

    // ServoMotor interface implementation
    void setup() override;
    void setPositionPercentage(int percentage) override;
    int getCurrentPercentage() const override;

private:
    Servo servoObject;              ///< Arduino Servo library instance
    int motorPin;                   ///< PWM pin connected to servo signal line
    int minAngleDegrees;            ///< Servo angle for 0% position
    int maxAngleDegrees;            ///< Servo angle for 100% position
    int currentMotorPercentage;     ///< Current target position (0-100%)
};

#endif // SERVO_MOTOR_IMPL_H