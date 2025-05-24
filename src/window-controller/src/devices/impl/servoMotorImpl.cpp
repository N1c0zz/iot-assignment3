#include "../api/ServoMotorImpl.h"

ServoMotorImpl::ServoMotorImpl(int pin, int minAngle, int maxAngle)
    : motorPin(pin),
      minAngleDegrees(minAngle),
      maxAngleDegrees(maxAngle),
      currentMotorPercentage(0) {}

void ServoMotorImpl::setup() {
    ServoMotor.attach(motorPin);
    // Imposta una posizione iniziale (es. chiusa)
    setPositionPercentage(0);
}

void ServoMotorImpl::setPositionPercentage(int percentage) {
    percentage = constrain(percentage, 0, 100);
    // Ottimizzazione: non muovere se già in posizione
    // if (percentage == currentMotorPercentage && ServoMotor.attached()) {
    //     return;
    // }

    int angle = map(percentage, 0, 100, minAngleDegrees, maxAngleDegrees);
    ServoMotor.write(angle);
    currentMotorPercentage = percentage;
}

int ServoMotorImpl::getCurrentPercentage() const {
    return currentMotorPercentage;
}