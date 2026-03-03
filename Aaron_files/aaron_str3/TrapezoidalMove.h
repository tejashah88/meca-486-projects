// TrapezoidalMove.h
// Unified stepper motor library with trapezoidal motion profiles.
// Supports both limit-switch (linear stage) and no-limit (rotating load) motors
// through a single motorInit() call.
//
// Usage — linear stage (with limit switches):
//   LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
//   MotorConfig motor;
//   motorInit(&motor, 1, true, 9, 10, 200, &lcd, 2, 3);
//
// Usage — rotating load (no limits):
//   MotorConfig motor;
//   motorInit(&motor, 2, false, 9, 10, 200, &lcd);

#ifndef TRAPEZOIDAL_MOVE_H
#define TRAPEZOIDAL_MOVE_H

#include <Arduino.h>
#include <LiquidCrystal.h>

// ── Motor configuration struct ────────────────────────────────────────────
struct MotorConfig {
  uint8_t id;           // user-assigned motor ID (printed in Serial output)
  bool    hasLimits;    // true = limit switches present; false = free-spinning load
  bool    invertDir;    // true = invert direction pin (swap CW/CCW due to motor wiring)

  // Pins
  int dirPin;
  int stepPin;
  int limitEndPin;      // end limit switch  (SENSOR_PIN_1) — ignored when hasLimits = false
  int limitHomePin;     // home limit switch (SENSOR_PIN_2) — ignored when hasLimits = false
  int stepsPerRev;
  float mmPerRev;       // lead screw pitch in mm/rev (e.g. 10.0 for 10mm pitch). 0 = unknown, skips mm output.
  float limitStopRevs;  // max stop distance in revs at maxRPS. At lower speeds, stop distance scales as v²/vmax² * limitStopRevs.
  float maxRPS;         // maximum operating speed (RPS). Used to compute the fixed limit decel rate.

  // State
  long position;        // current position in steps (0 = home or startup)
  long endPos;          // end limit position in steps   (hasLimits only)
  long axisLength;      // total axis travel in steps    (hasLimits only)
  float speedRPS;       // current speed in RPS (updated during moves)

  // Interrupt-driven limit flags — set by ISR on FALLING edge, cleared before each move
  volatile bool limitEndFlag;
  volatile bool limitHomeFlag;

  // Tachometer (driver Tach Out signal)
  int     tachPin;             // interrupt-capable pin (-1 if no tach)
  uint8_t tachPulsesPerRev;    // pulses per revolution (match driver setting)
  volatile uint32_t tachCount; // raw pulse count, incremented by ISR

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
               int            stepsPerRev,
               bool           invertDir         = false,
               LiquidCrystal* lcd              = nullptr,
               int            limitEndPin       = -1,
               int            limitHomePin      = -1,
               float          mmPerRev          = 0.0f,
               int            tachPin           = -1,
               uint8_t        tachPulsesPerRev  = 100,
               float          maxRPS            = 0.0f);

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

// ── Interrupt setup ───────────────────────────────────────────────────────
// Limit switches — call once after motorInit (hasLimits = true only).
// Pins must be interrupt-capable (e.g. 2, 3, 18-21 on Mega).
void attachLimitInterrupts(MotorConfig* m);

// Tachometer — call once after motorInit when tachPin >= 0.
void attachTachInterrupt(MotorConfig* m);
void resetTach(MotorConfig* m);
float getTachRevolutions(MotorConfig* m);               // revs since last resetTach
float getTachRPS(MotorConfig* m, uint16_t sampleMs = 50); // instantaneous RPS

// ── Homing & calibration (hasLimits = true only) ─────────────────────────
void homeAxis(MotorConfig* m, float slowRPS);
void findEnd(MotorConfig* m, float slowRPS);
void calibrateAxis(MotorConfig* m, float slowRPS);

// ── Position-based moves (hasLimits = true only) ─────────────────────────
void moveToHome(MotorConfig* m, float cruiseRPS);
void moveToEnd(MotorConfig* m, float cruiseRPS);

#endif
