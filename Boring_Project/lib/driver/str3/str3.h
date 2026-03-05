// str3.h
// STR3 stepper driver interface (Applied Motion Products).
// Owns pin configuration and direction control for a single axis.

#pragma once

#include "../../common/motor_types.h"

namespace STR3 {

  // Initialise all pins and zero all state fields.
  // limitEndPin / limitHomePin are ignored when hasLimits = false.
  // mmPerRev = 0 suppresses mm output. maxRPS = 0 disables limit decel math.
  void init(MotorConfig* m,
            uint8_t id,
            bool    hasLimits,
            int     dirPin,
            int     stepPin,
            int     stepsPerRev,
            bool    invertDir    = false,
            int     limitEndPin  = -1,
            int     limitHomePin = -1,
            float   mmPerRev     = 0.0f,
            float   maxRPS       = 0.0f);

  // Set the direction pin. forward = true moves toward the end limit in logical space.
  // Respects m->invertDir to compensate for physical wiring.
  // Exported so MotionProf and Cal can call it without accessing dirPin directly.
  void setDir(MotorConfig* m, bool forward);

} // namespace STR3
