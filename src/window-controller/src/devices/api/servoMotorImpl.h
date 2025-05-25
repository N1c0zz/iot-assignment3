#ifndef SERVO_MOTOR_IMPL_H
#define SERVO_MOTOR_IMPL_H

#include "ServoMotor.h" // The interface this class implements
#include <Arduino.h>    // For constrain(), map()
#include <Servo.h>      // Arduino's standard Servo library

/**
 * @class ServoMotorImpl
 * @brief Implements the ServoMotor interface using the standard Arduino Servo library.
 *
 * This class controls a hobby servo motor connected to a PWM pin on an Arduino board.
 * It maps a percentage input (0-100%) to a specified angular range (minAngle to maxAngle).
 */
class ServoMotorImpl : public ServoMotor {
public:
    /**
     * @brief Constructor for ServoMotorImpl.
     * @param pin The Arduino PWM pin to which the servo's signal line is connected.
     * @param minAngle The servo angle (in degrees) corresponding to 0% position.
     * @param maxAngle The servo angle (in degrees) corresponding to 100% position.
     */
    ServoMotorImpl(int pin, int minAngle, int maxAngle);

    void setup() override;
    void setPositionPercentage(int percentage) override;
    int getCurrentPercentage() const override;

private:
    Servo servoObject;          ///< Instance of the Arduino Servo library. (Was ServoMotor)
    int motorPin;               ///< Arduino pin connected to the servo's signal line.
    int minAngleDegrees;        ///< Servo angle (degrees) for 0% position.
    int maxAngleDegrees;        ///< Servo angle (degrees) for 100% position.
    int currentMotorPercentage; ///< Current target position stored as a percentage.
};

#endif // SERVO_MOTOR_IMPL_H