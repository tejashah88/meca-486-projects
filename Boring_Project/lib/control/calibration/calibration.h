// calibration.h
// Homing, axis calibration, and position-based moves for limit-switched axes.
// All functions require m->hasLimits = true.
// Depends on: STR3 (direction), MotionProf (profileMove), LCD (status updates), Tach (optional).

#pragma once

#include "../../common/motor_types.h"

namespace Cal {

  // Drive toward the home sensor and back off until clear. Sets m->position = 0.
  void homeAxis(MotorConfig* m, float slowRPS);

  // Drive toward the end sensor. Records m->endPos and m->axisLength.
  void findEnd(MotorConfig* m, float slowRPS);

  // Full calibration sequence: homeAxis then findEnd.
  // Prints axis length in steps, revs, and mm (if mmPerRev > 0).
  void calibrateAxis(MotorConfig* m, float slowRPS);

  // Move from current position to home (step 0) using a trapezoidal ramp.
  void moveToHome(MotorConfig* m, float cruiseRPS);

  // Move from current position to end limit using a trapezoidal ramp.
  void moveToEnd(MotorConfig* m, float cruiseRPS);

} // namespace Cal
