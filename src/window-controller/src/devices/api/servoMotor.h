#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

class servoMotor {
public:
    virtual ~servoMotor() {} // Distruttore virtuale

    virtual void setup() = 0;
    virtual void setPositionPercentage(int percentage) = 0; // Percentage 0-100
    virtual int getCurrentPercentage() const = 0;
    // virtual bool isAtTargetPosition() const = 0; // Opzionale, se necessario e implementabile
};

#endif // SERVO_MOTOR_H