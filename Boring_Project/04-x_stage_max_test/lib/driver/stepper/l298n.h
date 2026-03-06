// l298n.h
// ST Microelectronics L298N dual H-bridge motor driver.
// Drives a stepper motor via 4 phase pins (IN1–IN4) and 2 PWM enable pins (ENA/ENB).
// Wire motor coil A to OUT1/OUT2, coil B to OUT3/OUT4.

#pragma once

#include "step_hbridge_driver.h"

class L298N : public StepHBridgeDriver {
public:
  // in1Pin–in4Pin: phase outputs (IN1–IN4 on the board).
  // enaPin: ENA — PWM enable for coil A (OUT1/OUT2).
  // enbPin: ENB — PWM enable for coil B (OUT3/OUT4).
  // stepsPerRev: motor's full-step count (e.g. 200 for a 1.8° NEMA17).
  // dutyCycle: analogWrite value for current limiting (0–255); default 200 ≈ 78%.
  // halfStep: false = full-step (4 phases), true = half-step (8 phases).
  L298N(int in1Pin, int in2Pin, int in3Pin, int in4Pin,
        int enaPin, int enbPin,
        int stepsPerRev, uint8_t dutyCycle = 200, bool halfStep = false)
    : StepHBridgeDriver(in1Pin, in2Pin, in3Pin, in4Pin,
                        enaPin, enbPin, stepsPerRev, dutyCycle, halfStep) {}
};
