#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

class ServoMotor {
public:
    virtual ~ServoMotor() {} // Distruttore virtuale

    virtual void setup() = 0;
    virtual void setPositionPercentage(int percentage) = 0; // Percentage 0-100
    virtual int getCurrentPercentage() const = 0;
};

#endif // SERVO_MOTOR_H