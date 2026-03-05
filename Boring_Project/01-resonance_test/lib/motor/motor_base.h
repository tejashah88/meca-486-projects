// motor_base.h
// Base class for all stepper motor axes.
// Owns motion profile logic and delegates all hardware access to a StepperDriver.
// Subclasses add limit switches (LinearMotor) or nothing extra (RotationalMotor).

#pragma once

#include <Arduino.h>
#include "../driver/stepper_driver.h"

class MotorBase {
public:
  // Configure the motor axis. driver must outlive this object.
  // Calls driver->init(), caches stepsPerRev for fast access in tight loops.
  void init(uint8_t id, StepperDriver* driver);

  // Explicit 3-phase trapezoidal move (revolutions).
  // Sign of accelRevs sets direction; cruise and decel signs must match.
  // Accel rate: a = cruiseRPS² / (2 * |accelRevs|).
  void manualTrapMove(float accelRevs, float cruiseRevs, float decelRevs, float cruiseRPS);

  // Time-constrained trapezoidal move. Falls back to a symmetric triangle profile
  // when the distance cannot sustain a cruise phase.
  void autoTrapMove(float revolutions, float maxRPS, float totalTime);

  // Constant-velocity move — no accel/decel ramp. Intended for jogging or resonance testing.
  void spinRevs(float revolutions, float rps);

  // State getters used by Display and Serial output.
  uint8_t id()           const { return _id; }
  float   positionRevs() const { return (float)_position / _stepsPerRev; }
  float   speedRPS()     const { return _speedRPS; }
  bool    hasLimits()    const { return _hasLimits; }
  float   mmPerRev()     const { return _mmPerRev; }

  // Called by ISR stubs in linear_motor.cpp — must be public.
  void triggerEndLimit()  { _limitEndFlag  = true; }
  void triggerHomeLimit() { _limitHomeFlag = true; }

protected:
  uint8_t _id;
  bool    _hasLimits;
  int     _stepsPerRev;        // cached from driver->stepsPerRev() at init time
  int     _limitEndPin, _limitHomePin;   // -1 if unused (default for MotorBase)
  float   _mmPerRev, _maxRPS, _limitStopRevs;
  long    _position;
  float   _speedRPS;
  bool    _movingForward;      // tracks current direction for limitTriggered()
  volatile bool _limitEndFlag, _limitHomeFlag;

  StepperDriver* _driver;

  // Set direction on the driver and update _movingForward.
  void setDirection(bool forward);

private:
  // Direction-aware limit check using ISR-set flags.
  // Returns false when _hasLimits = false or when moving away from the triggered limit.
  bool limitTriggered();

  // Emergency decel to a stop using a fixed decel rate derived from _maxRPS.
  void runLimitDecel(float currentSpeedStepsPerSec, int8_t dir);

  // Core 3-phase step executor: accel → cruise → decel.
  void runTrapezoid(int aSteps, int cSteps, int dSteps,
                    float cruiseSpeed, float accelRate, float decelRate, int8_t dir);
};
