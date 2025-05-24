#ifndef SERVO_MOTOR_IMPL_H
#define SERVO_MOTOR_IMPL_H

#include "ServoMotor.h" // Interfaccia
#include <Arduino.h>
#include <Servo.h>      // Libreria specifica per questa implementazione

class ServoMotorImpl : public ServoMotor {
public:
    ServoMotorImpl(int pin, int minAngle, int maxAngle);
    void setup() override;
    void setPositionPercentage(int percentage) override;
    int getCurrentPercentage() const override;

private:
    Servo ServoMotor;
    int motorPin;
    int minAngleDegrees;
    int maxAngleDegrees;
    int currentMotorPercentage;
};

#endif // SERVO_MOTOR_IMPL_H