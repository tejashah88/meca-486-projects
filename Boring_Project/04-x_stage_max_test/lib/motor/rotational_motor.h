// rotational_motor.h
// Free-spinning rotational axis — no limit switches or calibration surface.
// Inherits all motion profile methods (manualTrapMove, autoTrapMove, spinRevs) from MotorBase.

#pragma once

#include "motor_base.h"

class RotationalMotor : public MotorBase {
public:
  // Configure the axis. Delegates directly to MotorBase::init(id, driver).
  void init(uint8_t id, StepperDriver* driver);
};
