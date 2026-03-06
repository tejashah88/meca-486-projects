// step_hbridge_driver.h
// StepHBridgeDriver: stepper motor driver for H-bridge ICs (e.g. L298N).
// Uses 4 phase pins (IN1–IN4) for full-step or half-step phase control.
// Two PWM-capable enable pins (ENA, ENB) allow current limiting via duty cycle.
// Extends StepperDriver directly — no step/dir pin; phase is tracked internally.

#pragma once

#include "../stepper_driver.h"

class StepHBridgeDriver : public StepperDriver {
public:
  // in1–in4: phase output pins (connect to IN1–IN4 on the H-bridge).
  // enaPin / enbPin: PWM enable pins for side A (IN1/IN2) and side B (IN3/IN4).
  // stepsPerRev: motor's full-step count (e.g. 200 for a 1.8° NEMA17).
  // dutyCycle: analogWrite value applied to enable pins when enabled (0–255).
  // halfStep: false = 4-phase full-step table; true = 8-phase half-step table.
  //           stepsPerRev() doubles automatically in half-step mode.
  StepHBridgeDriver(int in1Pin, int in2Pin, int in3Pin, int in4Pin,
                    int enaPin, int enbPin,
                    int stepsPerRev, uint8_t dutyCycle = 200,
                    bool halfStep = false);

  // Set all phase and enable pins to OUTPUT. Does not energise the coils.
  void init() override;

  // Advance one phase step in the current direction, write the 4 IN pins,
  // then hold for stepPeriodUs microseconds.
  void step(unsigned long stepPeriodUs) override;

  // Set the direction of phase advance. forward=true increments the phase index.
  void setDirection(bool forward) override;

  // Returns stepsPerRev passed to the constructor, doubled in half-step mode.
  int stepsPerRev() const override;

  // Apply dutyCycle to both enable pins via analogWrite.
  void enable()  override;

  // Write 0 to both enable pins (de-energise the bridge).
  void disable() override;

protected:
  int     _in1, _in2, _in3, _in4;
  int     _enaPin, _enbPin;
  int     _spr;
  uint8_t _dutyCycle;
  bool    _halfStep;
  bool    _forward;
  uint8_t _phase;   // current index into the active step table
};
