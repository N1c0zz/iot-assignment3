#ifndef SERVO_MOTOR_IMPL_H
#define SERVO_MOTOR_IMPL_H

#include "servoMotor.h" // Interfaccia
#include <Arduino.h>
#include <Servo.h>      // Libreria specifica per questa implementazione

class servoMotorImpl : public servoMotor {
public:
    servoMotorImpl(int pin, int minAngle, int maxAngle);
    void setup() override;
    void setPositionPercentage(int percentage) override;
    int getCurrentPercentage() const override;

private:
    Servo servoMotor;
    int motorPin;
    int minAngleDegrees;
    int maxAngleDegrees;
    int currentMotorPercentage;
};

#endif // SERVO_MOTOR_IMPL_H