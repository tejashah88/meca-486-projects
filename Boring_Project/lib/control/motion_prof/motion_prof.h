// motion_prof.h
// Trapezoidal motion profile control for STR3-driven stepper motors.
// Depends on: STR3 (direction), LimitSw (limit queries), LCD (post-move update), Tach (optional).

#pragma once

#include "../../common/motor_types.h"

namespace MotionProf {

  // 3-phase move with explicitly specified accel / cruise / decel distances.
  // Sign of accelRevs sets direction. cruiseRPS is the plateau speed.
  // All phases use the same motor, computed as: a = v² / (2 * d).
  void profileMove(MotorConfig* m,
                   float accelRevs, float cruiseRevs, float decelRevs,
                   float cruiseRPS);

  // Auto-calculated trapezoidal (or triangular) profile.
  // revolutions = signed total distance, maxRPS = speed cap, totalTime = wall-clock seconds.
  // Selects triangle profile when the distance cannot sustain a cruise phase.
  void trapezoidalMove(MotorConfig* m,
                       float revolutions, float maxRPS, float totalTime);

  // Constant velocity — no ramp. Intended for resonance testing.
  void rotate(MotorConfig* m, float revolutions, float rps);

} // namespace MotionProf
