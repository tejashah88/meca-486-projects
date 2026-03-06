// motor_base.cpp
// MotorBase: axis init and trapezoidal motion profile engine.
// All hardware pin access is delegated to the StepperDriver.

#include "lib/motor/motor_base.h"

// ── Init / direction ──────────────────────────────────────────────────────────

void MotorBase::init(uint8_t id, StepperDriver* driver) {
  _id            = id;
  _driver        = driver;
  _stepsPerRev   = driver->stepsPerRev();   // cache for fast loop access
  _hasLimits     = false;
  _limitEndPin   = -1;
  _limitHomePin  = -1;
  _mmPerRev      = 0.0f;
  _maxRPS        = 0.0f;
  _limitStopRevs = 2.0f;
  _position      = 0;
  _speedRPS      = 0.0f;
  _movingForward = true;
  _limitEndFlag  = false;
  _limitHomeFlag = false;

  driver->init();
}

void MotorBase::setDirection(bool forward) {
  _movingForward = forward;
  _driver->setDirection(forward);
}

// ── Private helpers ───────────────────────────────────────────────────────────

bool MotorBase::limitTriggered() {
  if (!_hasLimits) return false;
  return _movingForward ? _limitEndFlag : _limitHomeFlag;
}

// Fixed decel rate: a = (maxRPS * stepsPerRev)² / (2 * limitStopRevs * stepsPerRev)
// Stops the motor from _maxRPS within _limitStopRevs revolutions.
void MotorBase::runLimitDecel(float currentSpeedStepsPerSec, int8_t dir) {
  if (_limitStopRevs <= 0.0f || _maxRPS <= 0.0f) return;
  if (currentSpeedStepsPerSec < 1.0f) currentSpeedStepsPerSec = 1.0f;

  float maxSpeedSteps = _maxRPS * _stepsPerRev;
  int   maxStopSteps  = (int)(_limitStopRevs * _stepsPerRev);
  float decelRate     = (maxSpeedSteps * maxSpeedSteps) / (2.0f * maxStopSteps);
  int   stopSteps     = (int)(currentSpeedStepsPerSec * currentSpeedStepsPerSec / (2.0f * decelRate));
  if (stopSteps < 1) return;

  Serial.print("Limit decel: "); Serial.print(stopSteps);
  Serial.print(" steps ("); Serial.print((float)stopSteps / _stepsPerRev, 3);
  Serial.println(" revs)");

  for (int j = 0; j < stopSteps; j++) {
    float speed = sqrt(2.0f * decelRate * (stopSteps - j));
    if (speed < 1.0f) speed = 1.0f;
    unsigned long stepPeriod = (unsigned long)(1000000.0f / speed);
    _speedRPS = speed / _stepsPerRev;
    _driver->step(stepPeriod);
    _position += dir;
  }
  _speedRPS = 0;
}

// Core 3-phase step executor: accel → cruise → decel.
// Speed ramp uses v = sqrt(2 * a * distance).
void MotorBase::runTrapezoid(int accelSteps, int cruiseSteps, int decelSteps,
                              float cruiseSpeed, float accelRate, float decelRate,
                              int8_t dir) {
  unsigned long stepDelay;
  float speed;
  unsigned long startTime, accelEnd, cruiseEnd, decelEnd;

  // Pre-seed the flag for whichever limit we're moving toward.
  // The ISR fires on FALLING edge; if the switch is already held LOW, no edge fires.
  if (_hasLimits) {
    if (dir > 0) _limitEndFlag  = (_limitEndPin  >= 0 && digitalRead(_limitEndPin)  == LOW);
    else         _limitHomeFlag = (_limitHomePin >= 0 && digitalRead(_limitHomePin) == LOW);
  }

  // ── Accel ──────────────────────────────────────────────────────────────────
  startTime = micros();
  for (int i = 0; i < accelSteps; i++) {
    if (limitTriggered()) {
      Serial.print("Limit hit during accel at ");
      Serial.print(_speedRPS, 3); Serial.println(" RPS — decelling to stop.");
      runLimitDecel(_speedRPS * _stepsPerRev, dir);
      break;
    }
    speed = sqrt(2.0 * accelRate * i);
    if (speed < 1.0) speed = 1.0;
    stepDelay = (unsigned long)(1000000.0 / speed);
    _driver->step(stepDelay);
    _position += dir;
    _speedRPS = speed / _stepsPerRev;
  }
  accelEnd = micros();

  // ── Cruise ─────────────────────────────────────────────────────────────────
  if (cruiseSteps > 0) {
    stepDelay = (unsigned long)(1000000.0 / cruiseSpeed);
    _speedRPS = cruiseSpeed / _stepsPerRev;
    for (int i = 0; i < cruiseSteps; i++) {
      if (limitTriggered()) {
        Serial.print("Limit hit during cruise at ");
        Serial.print(_speedRPS, 3); Serial.println(" RPS — decelling to stop.");
        runLimitDecel(cruiseSpeed, dir);
        break;
      }
      _driver->step(stepDelay);
      _position += dir;
    }
  }
  cruiseEnd = micros();

  // ── Decel ──────────────────────────────────────────────────────────────────
  for (int i = 0; i < decelSteps; i++) {
    if (limitTriggered()) {
      Serial.print("Limit hit during decel at ");
      Serial.print(_speedRPS, 3); Serial.println(" RPS — decelling to stop.");
      runLimitDecel(_speedRPS * _stepsPerRev, dir);
      break;
    }
    speed = sqrt(2.0 * decelRate * (decelSteps - i));
    if (speed < 1.0) speed = 1.0;
    stepDelay = (unsigned long)(1000000.0 / speed);
    _driver->step(stepDelay);
    _position += dir;
    _speedRPS = speed / _stepsPerRev;
  }
  _speedRPS = 0;
  decelEnd = micros();

  // ── Serial timing report ───────────────────────────────────────────────────
  float tAccel  = (accelEnd  - startTime) / 1e6;
  float tCruise = (cruiseEnd - accelEnd)  / 1e6;
  float tDecel  = (decelEnd  - cruiseEnd) / 1e6;
  float tTotal  = (decelEnd  - startTime) / 1e6;

  float tAccelExp  = (accelSteps  > 0) ? cruiseSpeed / accelRate : 0;
  float tCruiseExp = (cruiseSteps > 0) ? (float)cruiseSteps / cruiseSpeed : 0;
  float tDecelExp  = (decelSteps  > 0) ? cruiseSpeed / decelRate : 0;
  float tTotalExp  = tAccelExp + tCruiseExp + tDecelExp;

  float commandedRevs = (float)(accelSteps + cruiseSteps + decelSteps) / _stepsPerRev;

  Serial.print("--- Motor "); Serial.print(_id); Serial.println(" Move Complete ---");
  Serial.print("Commanded: "); Serial.print(commandedRevs, 3); Serial.println(" rev");
  Serial.print("Position: "); Serial.println(_position);

  Serial.print("Accel:  Expected="); Serial.print(tAccelExp, 3);
  Serial.print("s, Actual="); Serial.print(tAccel, 3); Serial.println("s");
  Serial.print("Cruise: Expected="); Serial.print(tCruiseExp, 3);
  Serial.print("s, Actual="); Serial.print(tCruise, 3); Serial.println("s");
  Serial.print("Decel:  Expected="); Serial.print(tDecelExp, 3);
  Serial.print("s, Actual="); Serial.print(tDecel, 3); Serial.println("s");

  float err = tTotal - tTotalExp;
  Serial.print("Total:  Expected="); Serial.print(tTotalExp, 3);
  Serial.print("s, Actual="); Serial.print(tTotal, 3);
  Serial.print("s, Error="); Serial.print(err, 3);
  Serial.print("s (");
  Serial.print(tTotalExp > 0 ? (err / tTotalExp) * 100.0 : 0, 2);
  Serial.println("%)");
}

// ── Public move functions ─────────────────────────────────────────────────────

void MotorBase::manualTrapMove(float accelRevs, float cruiseRevs, float decelRevs,
                                float cruiseRPS) {
  setDirection(accelRevs > 0);
  int8_t dir = (accelRevs > 0) ? 1 : -1;

  int aSteps = (int)(abs(accelRevs)  * _stepsPerRev);
  int cSteps = (int)(abs(cruiseRevs) * _stepsPerRev);
  int dSteps = (int)(abs(decelRevs)  * _stepsPerRev);

  float cruiseSpeed = cruiseRPS * _stepsPerRev;
  float accelRate   = (cruiseSpeed * cruiseSpeed) / (2.0 * aSteps);
  float decelRate   = (cruiseSpeed * cruiseSpeed) / (2.0 * dSteps);

  Serial.print("--- Motor "); Serial.print(_id); Serial.println(" Profile Move ---");
  Serial.print("Accel="); Serial.print(accelRevs);
  Serial.print(", Cruise="); Serial.print(cruiseRevs);
  Serial.print(", Decel="); Serial.print(decelRevs);
  Serial.print(" rev, RPS="); Serial.println(cruiseRPS);

  runTrapezoid(aSteps, cSteps, dSteps, cruiseSpeed, accelRate, decelRate, dir);
}

void MotorBase::autoTrapMove(float revolutions, float maxRPS, float totalTime) {
  int    totalSteps = abs(revolutions * _stepsPerRev);
  setDirection(revolutions > 0);
  int8_t dir = (revolutions > 0) ? 1 : -1;

  float maxSpeed = maxRPS * _stepsPerRev;
  float tAccel   = totalTime - (totalSteps / maxSpeed);
  float tCruise  = totalTime - 2.0 * tAccel;

  if (tAccel <= 0 || tCruise < 0) {
    float peak      = (2.0 * totalSteps) / totalTime;
    float tRamp     = totalTime / 2.0;
    float a         = peak / tRamp;
    int   halfSteps = totalSteps / 2;
    runTrapezoid(halfSteps, 0, totalSteps - halfSteps, peak, a, a, dir);
  } else {
    float a    = maxSpeed / tAccel;
    int aSteps = (int)(0.5 * a * tAccel * tAccel);
    int cSteps = (int)(maxSpeed * tCruise);
    int dSteps = totalSteps - aSteps - cSteps;
    runTrapezoid(aSteps, cSteps, dSteps, maxSpeed, a, a, dir);
  }
}

void MotorBase::spinRevs(float revolutions, float rps) {
  int    total     = abs((int)(revolutions * _stepsPerRev));
  int8_t dir       = (revolutions > 0) ? 1 : -1;
  unsigned long stepPeriod = (unsigned long)(1000000.0 / (rps * _stepsPerRev));

  setDirection(revolutions > 0);
  _speedRPS = rps;
  for (int i = 0; i < total; i++) {
    _driver->step(stepPeriod);
    _position += dir;
  }
  _speedRPS = 0;
}
