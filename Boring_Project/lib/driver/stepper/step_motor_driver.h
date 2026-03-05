// step_motor_driver.h
// Common implementation for Applied Motion step/direction stepper drivers.
// Handles step pulse generation, direction control, and optional enable pin.
// Named driver classes (STR3, ST5, ST10) inherit from this.

#pragma once

#include "../stepper_driver.h"

class StepMotorDriver : public StepperDriver {
public:
  // dirPin / stepPin: digital output pins for direction and step signals.
  // stepsPerRev: microstep-per-revolution setting (matches DIP switches on the unit).
  // invertDir: true = invert direction pin (compensates for reversed motor wiring).
  // enablePin: optional active-HIGH enable pin; pass -1 (default) if unused.
  StepMotorDriver(int dirPin, int stepPin, int stepsPerRev,
                  bool invertDir = false, int enablePin = -1);

  // Set all pins to OUTPUT (and enable pin to OUTPUT if present).
  void init() override;

  // Toggle step pin: HIGH for halfPeriodUs, then LOW for halfPeriodUs.
  void step(unsigned long halfPeriodUs) override;

  // Write direction pin, accounting for invertDir.
  void setDirection(bool forward) override;

  int stepsPerRev() const override { return _stepsPerRev; }

  // Drive enable pin HIGH / LOW. No-op when enablePin = -1.
  void enable()  override;
  void disable() override;

protected:
  int  _dirPin, _stepPin, _stepsPerRev;
  bool _invertDir;
  int  _enablePin;
};
