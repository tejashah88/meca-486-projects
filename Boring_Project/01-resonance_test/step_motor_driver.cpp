// step_motor_driver.cpp
// StepMotorDriver: step/direction pin control for Applied Motion drivers.

#include "lib/driver/stepper/step_motor_driver.h"

StepMotorDriver::StepMotorDriver(int dirPin, int stepPin, int enablePin,
                                  int stepsPerRev, bool invertDir)
  : _dirPin(dirPin), _stepPin(stepPin), _enablePin(enablePin),
    _stepsPerRev(stepsPerRev), _invertDir(invertDir) {}

void StepMotorDriver::init() {
  pinMode(_dirPin,  OUTPUT);
  pinMode(_stepPin, OUTPUT);
  if (_enablePin >= 0) pinMode(_enablePin, OUTPUT);
}

void StepMotorDriver::step(unsigned long halfPeriodUs) {
  digitalWrite(_stepPin, HIGH);
  delayMicroseconds(halfPeriodUs);
  digitalWrite(_stepPin, LOW);
  delayMicroseconds(halfPeriodUs);
}

void StepMotorDriver::setDirection(bool forward) {
  digitalWrite(_dirPin, (forward ^ _invertDir) ? LOW : HIGH);
}

void StepMotorDriver::enable() {
  if (_enablePin >= 0) digitalWrite(_enablePin, HIGH);
}

void StepMotorDriver::disable() {
  if (_enablePin >= 0) digitalWrite(_enablePin, LOW);
}
