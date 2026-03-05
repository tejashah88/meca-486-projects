// motor_types.h
// Shared data model for all motor driver and control modules.
// This header defines MotorConfig and nothing else — no functions live here.

#pragma once

#include <Arduino.h>

struct MotorConfig {
  // ── Identity ──────────────────────────────────────────────────────────────
  uint8_t id;         // user-assigned motor ID (printed in Serial output)
  bool    hasLimits;  // true = limit switches present; false = free-spinning load
  bool    invertDir;  // true = invert direction pin (swap CW/CCW due to motor wiring)

  // ── Hardware pins (set once in STR3::init, never changed at runtime) ──────
  int dirPin;
  int stepPin;
  int limitEndPin;   // end limit switch  — -1 if unused
  int limitHomePin;  // home limit switch — -1 if unused

  // ── Motion parameters ─────────────────────────────────────────────────────
  int   stepsPerRev;
  float mmPerRev;       // lead screw pitch in mm/rev; 0 = unknown, skips mm output
  float limitStopRevs;  // max stop distance in revs at maxRPS; scales as v²/vmax² at lower speeds
  float maxRPS;         // operating ceiling; used to compute fixed limit decel rate

  // ── Runtime state (mutated during moves) ──────────────────────────────────
  long  position;    // current position in steps (0 = home or power-on)
  long  endPos;      // end limit position in steps   (hasLimits only)
  long  axisLength;  // total axis travel in steps    (hasLimits only)
  float speedRPS;    // current speed in RPS (updated during moves, 0 when stopped)

  // ── Interrupt-driven flags (set by ISR on FALLING edge, cleared before moves)
  volatile bool limitEndFlag;
  volatile bool limitHomeFlag;
};
