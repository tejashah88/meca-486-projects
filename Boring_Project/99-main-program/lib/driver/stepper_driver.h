// stepper_driver.h
// Abstract interface for all stepper motor drivers.
// Concrete subclasses own pin configuration and the step/direction protocol.
// MotorBase holds a StepperDriver* and delegates all hardware access through it.

#pragma once

#include <Arduino.h>

class StepperDriver {
public:
  // Configure hardware pins. Called once by MotorBase::init().
  virtual void init() = 0;

  // Pulse the step pin once. halfPeriodUs is the HIGH/LOW phase duration in microseconds.
  // The motion profile computes timing; the driver handles the actual pin toggle.
  virtual void step(unsigned long halfPeriodUs) = 0;

  // Set direction. forward = true moves toward the end limit in logical space.
  virtual void setDirection(bool forward) = 0;

  // Microsteps per revolution — matches the DIP-switch setting on this driver unit.
  virtual int stepsPerRev() const = 0;

  // Enable / disable motor power. Default no-ops for drivers without an enable pin (e.g. STR3).
  virtual void enable()  {}
  virtual void disable() {}
};
