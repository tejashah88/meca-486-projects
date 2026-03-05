// limit_sw.h
// Limit switch ISR registration and live state queries.
// Supports up to 4 motors (MAX_LIMIT_MOTORS) via a fixed slot-table of ISR stubs.

#pragma once

#include "../../common/motor_types.h"

namespace LimitSw {

  // Register FALLING-edge interrupts for m->limitEndPin and m->limitHomePin.
  // Finds a free slot in the internal stub table (max 4 motors).
  // Pre-clears both flags before attaching. Requires m->hasLimits = true.
  // Replaces attachLimitInterrupts().
  void attach(MotorConfig* m);

  // Detach interrupts and free the ISR slot. Safe to call even if m was never registered.
  void detach(MotorConfig* m);

  // Read live pin state (used by MotionProf to pre-seed flags before a move).
  // Returns true if the end/home pin is currently LOW (sensor active).
  // Returns false when m->limitEndPin / m->limitHomePin is -1.
  bool endActive(MotorConfig* m);
  bool homeActive(MotorConfig* m);

} // namespace LimitSw
