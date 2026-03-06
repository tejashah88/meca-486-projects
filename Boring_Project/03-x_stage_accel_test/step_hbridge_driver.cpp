// step_hbridge_driver.cpp
// StepHBridgeDriver: phase-based stepper control for H-bridge drivers (L298N, etc.)

#include "lib/driver/stepper/step_hbridge_driver.h"

// ── Phase tables ──────────────────────────────────────────────────────────────

// Full-step: 4 phases. Each row = {IN1, IN2, IN3, IN4}.
static const uint8_t FULL_STEPS[4][4] = {
  {HIGH, LOW,  HIGH, LOW },
  {LOW,  HIGH, HIGH, LOW },
  {LOW,  HIGH, LOW,  HIGH},
  {HIGH, LOW,  LOW,  HIGH},
};

// Half-step: 8 phases. Interleaves single-coil and dual-coil states.
static const uint8_t HALF_STEPS[8][4] = {
  {HIGH, LOW,  LOW,  LOW },
  {HIGH, LOW,  HIGH, LOW },
  {LOW,  LOW,  HIGH, LOW },
  {LOW,  HIGH, HIGH, LOW },
  {LOW,  HIGH, LOW,  LOW },
  {LOW,  HIGH, LOW,  HIGH},
  {LOW,  LOW,  LOW,  HIGH},
  {HIGH, LOW,  LOW,  HIGH},
};

// ── Constructor ───────────────────────────────────────────────────────────────

StepHBridgeDriver::StepHBridgeDriver(int in1Pin, int in2Pin, int in3Pin, int in4Pin,
                                     int enaPin, int enbPin,
                                     int stepsPerRev, uint8_t dutyCycle, bool halfStep)
  : _in1(in1Pin), _in2(in2Pin), _in3(in3Pin), _in4(in4Pin),
    _enaPin(enaPin), _enbPin(enbPin),
    _spr(stepsPerRev), _dutyCycle(dutyCycle),
    _halfStep(halfStep), _forward(true), _phase(0) {}

// ── StepperDriver interface ───────────────────────────────────────────────────

void StepHBridgeDriver::init() {
  pinMode(_in1, OUTPUT);
  pinMode(_in2, OUTPUT);
  pinMode(_in3, OUTPUT);
  pinMode(_in4, OUTPUT);
  pinMode(_enaPin, OUTPUT);
  pinMode(_enbPin, OUTPUT);
}

void StepHBridgeDriver::setDirection(bool forward) {
  _forward = forward;
}

int StepHBridgeDriver::stepsPerRev() const {
  return _halfStep ? _spr * 2 : _spr;
}

void StepHBridgeDriver::enable() {
  analogWrite(_enaPin, _dutyCycle);
  analogWrite(_enbPin, _dutyCycle);
  // Energize coils to phase 0 so the rotor aligns to a known position.
  // Without this, all IN pins remain LOW (L298N brake mode) and the rotor
  // is unaligned when stepping begins, causing buzz instead of rotation.
  const uint8_t* row = _halfStep ? HALF_STEPS[_phase] : FULL_STEPS[_phase];
  digitalWrite(_in1, row[0]);
  digitalWrite(_in2, row[1]);
  digitalWrite(_in3, row[2]);
  digitalWrite(_in4, row[3]);
}

void StepHBridgeDriver::disable() {
  analogWrite(_enaPin, 0);
  analogWrite(_enbPin, 0);
}

void StepHBridgeDriver::step(unsigned long stepPeriodUs) {
  const int tableSize = _halfStep ? 8 : 4;

  if (_forward) {
    _phase = (_phase + 1) % tableSize;
  } else {
    _phase = (_phase == 0) ? (tableSize - 1) : (_phase - 1);
  }

  const uint8_t* row = _halfStep ? HALF_STEPS[_phase] : FULL_STEPS[_phase];
  digitalWrite(_in1, row[0]);
  digitalWrite(_in2, row[1]);
  digitalWrite(_in3, row[2]);
  digitalWrite(_in4, row[3]);

  delayMicroseconds(stepPeriodUs);
}
