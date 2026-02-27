// TrapezoidalMove.h
// Unified stepper motor library with trapezoidal motion profiles.
// Supports both limit-switch (linear stage) and no-limit (rotating load) motors
// through a single motorInit() call.
//
// Usage — linear stage (with limit switches):
//   LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
//   MotorConfig motor;
//   motorInit(&motor, 1, true, 9, 10, 22, 200, &lcd, 2, 3);
//
// Usage — rotating load (no limits):
//   MotorConfig motor;
//   motorInit(&motor, 2, false, 9, 10, 22, 200, &lcd);

#ifndef TRAPEZOIDAL_MOVE_H
#define TRAPEZOIDAL_MOVE_H

#include <Arduino.h>
#include <LiquidCrystal.h>

// ── Motor configuration struct ────────────────────────────────────────────
struct MotorConfig {
  uint8_t id;           // user-assigned motor ID (printed in Serial output)
  bool    hasLimits;    // true = limit switches present; false = free-spinning load

  // Pins
  int dirPin;
  int stepPin;
  int buttonPin;
  int limitEndPin;      // end limit switch  (SENSOR_PIN_1) — ignored when hasLimits = false
  int limitHomePin;     // home limit switch (SENSOR_PIN_2) — ignored when hasLimits = false
  int stepsPerRev;
  float limitStopRevs;  // max revs for soft stop when limit hit (e.g. 2.0). 0 = stop immediately. Actual distance = min(required from speed, this cap).

  // State
  long position;        // current position in steps (0 = home or startup)
  long endPos;          // end limit position in steps   (hasLimits only)
  long axisLength;      // total axis travel in steps    (hasLimits only)
  float speedRPS;       // current speed in RPS (updated during moves)

  LiquidCrystal* lcd;   // pointer to LCD instance; pass nullptr for no LCD
};

// ── Initialization ────────────────────────────────────────────────────────
// Configures all pins and zeroes state.
// limitEndPin / limitHomePin only required when hasLimits = true.
void motorInit(MotorConfig* m,
               uint8_t        id,
               bool           hasLimits,
               int            dirPin,
               int            stepPin,
               int            buttonPin,
               int            stepsPerRev,
               LiquidCrystal* lcd         = nullptr,
               int            limitEndPin  = -1,
               int            limitHomePin = -1);

// ── LCD ───────────────────────────────────────────────────────────────────
void updateLCD(MotorConfig* m);

// ── Move functions ────────────────────────────────────────────────────────
// accel/cruise/decel distances in revs, cruise speed in RPS.
// Sign of accelRevs sets direction.
void profileMove(MotorConfig* m,
                 float accelRevs, float cruiseRevs, float decelRevs,
                 float cruiseRPS);

// Total distance, speed cap, and total time. Auto-selects trapezoid or triangle.
void trapezoidalMove(MotorConfig* m,
                     float revolutions, float maxRPS, float totalTime);

// Constant velocity — no ramp. Use for resonance testing.
void rotate(MotorConfig* m, float revolutions, float rps);

// ── Homing & calibration (hasLimits = true only) ─────────────────────────
void homeAxis(MotorConfig* m, float slowRPS);
void findEnd(MotorConfig* m, float slowRPS);
void calibrateAxis(MotorConfig* m, float slowRPS);

// ── Position-based moves (hasLimits = true only) ─────────────────────────
void moveToHome(MotorConfig* m, float cruiseRPS);
void moveToEnd(MotorConfig* m, float cruiseRPS);

#endif
