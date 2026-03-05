// display.h
// Motor status rendering for the LCD.
// Reads MotorConfig state and calls LCD driver primitives.
// The LCD must be initialised via LCD::init() before calling these functions.

#pragma once

#include "../../common/motor_types.h"

namespace Display {

  // Render motor position (row 0) and limit/status (row 1).
  // Rate-limited to 10 Hz internally. No-op if LCD not initialised.
  void updateMotor(MotorConfig* m);

} // namespace Display
