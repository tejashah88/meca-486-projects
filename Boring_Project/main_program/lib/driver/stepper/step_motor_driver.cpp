// step_motor_driver.cpp
// StepMotorDriver: step/direction pin control for Applied Motion drivers.

#include "step_motor_driver.h"

StepMotorDriver::StepMotorDriver(int dirPin, int stepPin, int stepsPerRev,
                                  bool invertDir, int enablePin)
  : _dirPin(dirPin), _stepPin(stepPin), _stepsPerRev(stepsPerRev),
    _invertDir(invertDir), _enablePin(enablePin) {}

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
