#include "../api/ServoMotorImpl.h" // Corresponding header

ServoMotorImpl::ServoMotorImpl(int pin, int minAngle, int maxAngle)
    : motorPin(pin),
      minAngleDegrees(minAngle),
      maxAngleDegrees(maxAngle),
      currentMotorPercentage(0) // Initial percentage is 0
{}

void ServoMotorImpl::setup() {
    servoObject.attach(motorPin); // Attach the servo to the specified pin
    // Set an initial position, typically closed or a known safe state.
    setPositionPercentage(0);
}

void ServoMotorImpl::setPositionPercentage(int percentage) {
    // Constrain the input percentage to the valid range of 0-100.
    percentage = constrain(percentage, 0, 100);

    // Optimization: if the servo is already at the target percentage and attached, do nothing.
    if (percentage == currentMotorPercentage && servoObject.attached()) {
         return;
    }

    // Map the percentage (0-100) to the defined angular range for the servo.
    int angle = map(percentage, 0, 100, minAngleDegrees, maxAngleDegrees);

    // Command the servo to move to the calculated angle.
    servoObject.write(angle);

    // Update the stored current position.
    currentMotorPercentage = percentage;
}

int ServoMotorImpl::getCurrentPercentage() const {
    return currentMotorPercentage;
}