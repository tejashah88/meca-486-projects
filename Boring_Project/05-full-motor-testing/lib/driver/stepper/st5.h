// st5.h
// Applied Motion Products ST5-S DC Advanced Microstep Driver.
// Differential Step/Dir (STEP+/STEP-, DIR+/DIR-) + enable pins (EN+/EN-).
// Wire STEP- and DIR- to GND; connect Arduino outputs to STEP+ and DIR+.
// enablePin drives EN+ (active HIGH to enable motor power).

#pragma once

#include "step_motor_driver.h"

class ST5 : public StepMotorDriver {
public:
  // dirPin: DIR+ output. stepPin: STEP+ output. enablePin: EN+ output.
  // stepsPerRev: set by DIP switches on the driver.
  ST5(int dirPin, int stepPin, int enablePin, int stepsPerRev, bool invertDir = false)
    : StepMotorDriver(dirPin, stepPin, enablePin, stepsPerRev, invertDir) {}
};
