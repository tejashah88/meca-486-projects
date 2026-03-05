// st10.h
// Applied Motion Products ST10-S DC Advanced Microstep Driver.
// Same step/dir + enable interface as ST5-S; higher current rating (10A peak).
// Wire STEP- and DIR- to GND; connect Arduino outputs to STEP+ and DIR+.
// enablePin drives EN+ (active HIGH to enable motor power).

#pragma once

#include "step_motor_driver.h"

class ST10 : public StepMotorDriver {
public:
  // dirPin: DIR+ output. stepPin: STEP+ output. enablePin: EN+ output.
  // stepsPerRev: set by DIP switches on the driver.
  ST10(int dirPin, int stepPin, int stepsPerRev, bool invertDir, int enablePin)
    : StepMotorDriver(dirPin, stepPin, stepsPerRev, invertDir, enablePin) {}
};
