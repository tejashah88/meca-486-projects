// linear_motor.cpp
// LinearMotor: limit switch ISR management, homing, and calibration.

#include "linear_motor.h"
#include "../control/display/display.h"

// ── ISR slot table ────────────────────────────────────────────────────────────
// Stores up to 4 LinearMotor pointers. Each slot owns one end-ISR and one home-ISR stub.
// Stubs call the public trigger methods defined on MotorBase.

namespace {

  static const int MAX_SLOTS = 4;
  static LinearMotor* _motors[MAX_SLOTS] = {nullptr, nullptr, nullptr, nullptr};

  static void endISR0()  { if (_motors[0]) _motors[0]->triggerEndLimit(); }
  static void homeISR0() { if (_motors[0]) _motors[0]->triggerHomeLimit(); }
  static void endISR1()  { if (_motors[1]) _motors[1]->triggerEndLimit(); }
  static void homeISR1() { if (_motors[1]) _motors[1]->triggerHomeLimit(); }
  static void endISR2()  { if (_motors[2]) _motors[2]->triggerEndLimit(); }
  static void homeISR2() { if (_motors[2]) _motors[2]->triggerHomeLimit(); }
  static void endISR3()  { if (_motors[3]) _motors[3]->triggerEndLimit(); }
  static void homeISR3() { if (_motors[3]) _motors[3]->triggerHomeLimit(); }

  typedef void (*IsrFunc)();
  static const IsrFunc endISRs[]  = {endISR0,  endISR1,  endISR2,  endISR3};
  static const IsrFunc homeISRs[] = {homeISR0, homeISR1, homeISR2, homeISR3};

} // anonymous namespace

// ── Init ──────────────────────────────────────────────────────────────────────

void LinearMotor::init(uint8_t id, StepperDriver* driver,
                        int limitEndPin, int limitHomePin, float mmPerRev, float maxRPS) {
  MotorBase::init(id, driver);
  _hasLimits    = true;
  _limitEndPin  = limitEndPin;
  _limitHomePin = limitHomePin;
  _mmPerRev     = mmPerRev;
  _maxRPS       = maxRPS;
  _endPos       = 0;
  _axisLength   = 0;

  if (limitEndPin  >= 0) pinMode(limitEndPin,  INPUT_PULLUP);
  if (limitHomePin >= 0) pinMode(limitHomePin, INPUT_PULLUP);
}

// ── Limit switch management ───────────────────────────────────────────────────

void LinearMotor::enableLimits() {
  int slot = -1;
  for (int i = 0; i < MAX_SLOTS; i++) {
    if (_motors[i] == nullptr) { slot = i; break; }
  }
  if (slot < 0) {
    Serial.println("LinearMotor::enableLimits: no free ISR slots.");
    return;
  }

  _motors[slot] = this;
  if (_limitEndPin  >= 0) attachInterrupt(digitalPinToInterrupt(_limitEndPin),  endISRs[slot],  FALLING);
  if (_limitHomePin >= 0) attachInterrupt(digitalPinToInterrupt(_limitHomePin), homeISRs[slot], FALLING);

  _limitEndFlag  = false;
  _limitHomeFlag = false;

  Serial.print("LinearMotor: ISRs attached for motor "); Serial.print(_id);
  Serial.print(" (slot "); Serial.print(slot); Serial.println(")");
}

void LinearMotor::disableLimits() {
  for (int i = 0; i < MAX_SLOTS; i++) {
    if (_motors[i] == this) {
      if (_limitEndPin  >= 0) detachInterrupt(digitalPinToInterrupt(_limitEndPin));
      if (_limitHomePin >= 0) detachInterrupt(digitalPinToInterrupt(_limitHomePin));
      _motors[i] = nullptr;
      return;
    }
  }
}

bool LinearMotor::atEnd()  const { return _limitEndPin  >= 0 && digitalRead(_limitEndPin)  == LOW; }
bool LinearMotor::atHome() const { return _limitHomePin >= 0 && digitalRead(_limitHomePin) == LOW; }

// ── Private creep helpers ─────────────────────────────────────────────────────

void LinearMotor::creepUntilSensor(int sensorPin, int8_t dir, float rps) {
  unsigned long halfPeriod = (unsigned long)(500000.0 / (rps * _stepsPerRev));
  while (digitalRead(sensorPin) == HIGH) {
    _driver->step(halfPeriod);
    _position += dir;
  }
}

void LinearMotor::creepUntilSensorClear(int sensorPin, int8_t dir, float rps) {
  unsigned long halfPeriod = (unsigned long)(500000.0 / (rps * _stepsPerRev));
  while (digitalRead(sensorPin) == LOW) {
    _driver->step(halfPeriod);
    _position += dir;
  }
}

// ── Calibration ───────────────────────────────────────────────────────────────

void LinearMotor::findHome(float slowRPS) {
  Serial.print("--- Motor "); Serial.print(_id); Serial.println(" Homing ---");

  if (digitalRead(_limitHomePin) == LOW) {
    Serial.println("Already on home sensor — backing off until clear.");
    setDirection(true);  // toward end = away from home
    creepUntilSensorClear(_limitHomePin, 1, slowRPS);
  } else {
    setDirection(false);  // toward home
    creepUntilSensor(_limitHomePin, -1, slowRPS);
    Serial.println("Home sensor detected — backing off until clear.");
    setDirection(true);   // away from home
    creepUntilSensorClear(_limitHomePin, 1, slowRPS);
  }
  _position = 0;
  Serial.println("Home set. Position = 0.");
  Display::renderMotorInfo(*this);
}

void LinearMotor::findEnd(float slowRPS) {
  Serial.print("--- Motor "); Serial.print(_id); Serial.println(" Finding End ---");

  if (digitalRead(_limitEndPin) == LOW) {
    Serial.println("Already at end.");
    _endPos     = _position;
    _axisLength = _endPos;
    Display::renderMotorInfo(*this);
    return;
  }
  setDirection(true);  // toward end
  creepUntilSensor(_limitEndPin, 1, slowRPS);
  _endPos     = _position;
  _axisLength = _endPos;
  Serial.print("End found at "); Serial.print(_endPos);
  Serial.print(" steps ("); Serial.print((float)_axisLength / _stepsPerRev, 3);
  Serial.println(" revs)");
  Display::renderMotorInfo(*this);
}

void LinearMotor::calibrate(float slowRPS) {
  findHome(slowRPS);
  findEnd(slowRPS);

  float stepRevs = (float)_axisLength / _stepsPerRev;
  Serial.print("--- Motor "); Serial.print(_id); Serial.println(" Calibration Complete ---");
  Serial.print("Axis (steps): "); Serial.print(_axisLength); Serial.print(" steps | ");
  Serial.print(stepRevs, 3); Serial.print(" revs");
  if (_mmPerRev > 0.0f) {
    Serial.print(" | "); Serial.print(stepRevs * _mmPerRev, 2); Serial.print(" mm");
  }
  Serial.println();
}

void LinearMotor::goHome(float cruiseRPS) {
  float totalRevs = (float)_position / _stepsPerRev;
  if (abs(totalRevs) < 0.01f) { Serial.println("Already at home."); return; }
  float ramp   = min(2.0f, abs(totalRevs) / 3.0f);
  float cruise = abs(totalRevs) - 2.0f * ramp;
  manualTrapMove(-ramp, -cruise, -ramp, cruiseRPS);
}

void LinearMotor::goToEnd(float cruiseRPS) {
  float totalRevs = (float)(_endPos - _position) / _stepsPerRev;
  if (abs(totalRevs) < 0.01f) { Serial.println("Already at end."); return; }
  float ramp   = min(2.0f, abs(totalRevs) / 3.0f);
  float cruise = abs(totalRevs) - 2.0f * ramp;
  manualTrapMove(ramp, cruise, ramp, cruiseRPS);
}
