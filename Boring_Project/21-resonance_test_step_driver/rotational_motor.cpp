// rotational_motor.cpp

#include "lib/motor/rotational_motor.h"

void RotationalMotor::init(uint8_t id, StepperDriver* driver) {
  MotorBase::init(id, driver);
}
