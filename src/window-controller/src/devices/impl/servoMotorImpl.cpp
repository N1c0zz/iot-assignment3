#include "../api/ServoMotorImpl.h"
#include "config/config.h"

ServoMotorImpl::ServoMotorImpl(int pin, int minAngle, int maxAngle)
    : motorPin(pin)
    , minAngleDegrees(minAngle)
    , maxAngleDegrees(maxAngle)
    , currentMotorPercentage(0)
{
    // Initialization done in setup()
}

void ServoMotorImpl::setup() {
    // Attach servo to PWM pin
    servoObject.attach(motorPin);
    
    // Set initial position to closed (0%)
    setPositionPercentage(0);
}

void ServoMotorImpl::setPositionPercentage(int percentage) {
    // Constrain input to valid range
    percentage = constrain(percentage, 0, 100);

    // Optimization: skip if already at target position and servo is attached
    if (percentage == currentMotorPercentage && servoObject.attached()) {
        return;
    }

    // Dead zone filtering: ignore small changes to prevent micro-movements
    if (abs(percentage - currentMotorPercentage) < MANUAL_PERCENTAGE_CHANGE_THRESHOLD) {
        return;
    }

    // Map percentage to servo angle within configured range
    int targetAngle = map(percentage, 0, 100, minAngleDegrees, maxAngleDegrees);

    // Command servo to move to calculated angle
    servoObject.write(targetAngle);

    // Update internal position tracking
    currentMotorPercentage = percentage;
}

int ServoMotorImpl::getCurrentPercentage() const {
    return currentMotorPercentage;
}