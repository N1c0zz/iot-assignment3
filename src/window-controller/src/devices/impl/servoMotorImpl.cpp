#include "../api/servoMotorImpl.h"

servoMotorImpl::servoMotorImpl(int pin, int minAngle, int maxAngle)
    : motorPin(pin),
      minAngleDegrees(minAngle),
      maxAngleDegrees(maxAngle),
      currentMotorPercentage(0) {}

void servoMotorImpl::setup() {
    servoMotor.attach(motorPin);
    // Imposta una posizione iniziale (es. chiusa)
    setPositionPercentage(0);
}

void servoMotorImpl::setPositionPercentage(int percentage) {
    percentage = constrain(percentage, 0, 100);
    // Ottimizzazione: non muovere se già in posizione
    // if (percentage == currentMotorPercentage && servoMotor.attached()) {
    //     return;
    // }

    int angle = map(percentage, 0, 100, minAngleDegrees, maxAngleDegrees);
    servoMotor.write(angle);
    currentMotorPercentage = percentage;
}

int servoMotorImpl::getCurrentPercentage() const {
    return currentMotorPercentage;
}