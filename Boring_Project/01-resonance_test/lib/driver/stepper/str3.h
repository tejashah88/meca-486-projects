// str3.h
// Applied Motion Products STR3 step motor driver.
// Single-ended Step/Dir interface (SW11 OFF = Step/Dir mode). No enable pin.

#pragma once

#include "step_motor_driver.h"

class STR3 : public StepMotorDriver {
public:
  // dirPin: direction output. stepPin: step pulse output.
  // stepsPerRev: set by SW5-SW8 on the driver (200–20000).
  // invertDir: true = invert direction logic (compensates reversed motor wiring).
  STR3(int dirPin, int stepPin, int stepsPerRev, bool invertDir = false)
    : StepMotorDriver(dirPin, stepPin, stepsPerRev, invertDir) {}
};
