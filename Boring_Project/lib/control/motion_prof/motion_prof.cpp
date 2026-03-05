// motion_prof.cpp
// Trapezoidal motion profile engine.
// runTrapezoid, runLimitDecel, and limitTriggered are private to this translation unit.

#include "motion_prof.h"
#include "../../driver/str3/str3.h"
#include "../../driver/limit_sw/limit_sw.h"

// ── Private helpers ───────────────────────────────────────────────────────────

namespace {

// Direction-aware limit check using ISR-set flags.
// Only blocks movement toward the triggered limit; allows reverse away.
// Always returns false when hasLimits = false.
static bool limitTriggered(MotorConfig* m) {
  if (!m->hasLimits) return false;
  bool movingTowardEnd = (digitalRead(m->dirPin) == LOW) ^ m->invertDir;
  if (movingTowardEnd) return m->limitEndFlag;
  else                 return m->limitHomeFlag;
}

// Fixed decel rate: a = (maxRPS * stepsPerRev)² / (2 * limitStopRevs * stepsPerRev)
// Stops the motor from maxRPS within limitStopRevs revolutions.
// At current (lower) speed, stop distance = v²/(2a) ≤ limitStopRevs.
static void runLimitDecel(MotorConfig* m, float currentSpeedStepsPerSec, int8_t dir) {
  if (m->limitStopRevs <= 0.0f || m->maxRPS <= 0.0f) return;
  if (currentSpeedStepsPerSec < 1.0f) currentSpeedStepsPerSec = 1.0f;

  float maxSpeedSteps = m->maxRPS * m->stepsPerRev;
  int   maxStopSteps  = (int)(m->limitStopRevs * m->stepsPerRev);
  float decelRate     = (maxSpeedSteps * maxSpeedSteps) / (2.0f * maxStopSteps);
  int   stopSteps     = (int)(currentSpeedStepsPerSec * currentSpeedStepsPerSec / (2.0f * decelRate));
  if (stopSteps < 1) return;

  Serial.print("Limit decel: "); Serial.print(stopSteps);
  Serial.print(" steps ("); Serial.print((float)stopSteps / m->stepsPerRev, 3);
  Serial.println(" revs)");

  for (int j = 0; j < stopSteps; j++) {
    float speed = sqrt(2.0f * decelRate * (stopSteps - j));
    if (speed < 1.0f) speed = 1.0f;
    unsigned long stepDelay = (unsigned long)(1000000.0f / speed);
    m->speedRPS = speed / m->stepsPerRev;
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(stepDelay / 2);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(stepDelay / 2);
    m->position += dir;
  }
  m->speedRPS = 0;
}

// Core 3-phase step executor: accel → cruise → decel.
// Speed ramp uses v = sqrt(2 * a * distance).
static void runTrapezoid(MotorConfig* m,
                         int accelSteps, int cruiseSteps, int decelSteps,
                         float cruiseSpeed, float accelRate, float decelRate,
                         int8_t dir) {
  unsigned long stepDelay;
  float speed;
  unsigned long startTime, accelEnd, cruiseEnd, decelEnd;

  // Pre-seed the flag for whichever limit we're moving toward.
  // The ISR fires on FALLING edge; if the switch is already held LOW, no edge fires.
  if (m->hasLimits) {
    if (dir > 0) m->limitEndFlag  = LimitSw::endActive(m);
    else         m->limitHomeFlag = LimitSw::homeActive(m);
  }

  // ── Accel ──────────────────────────────────────────────────────────────────
  startTime = micros();
  for (int i = 0; i < accelSteps; i++) {
    if (limitTriggered(m)) {
      Serial.print("Limit hit during accel at ");
      Serial.print(m->speedRPS, 3); Serial.println(" RPS — decelling to stop.");
      runLimitDecel(m, m->speedRPS * m->stepsPerRev, dir);
      break;
    }
    speed = sqrt(2.0 * accelRate * i);
    if (speed < 1.0) speed = 1.0;
    stepDelay = (unsigned long)(1000000.0 / speed);
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(stepDelay / 2);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(stepDelay / 2);
    m->position += dir;
    m->speedRPS = speed / m->stepsPerRev;
  }
  accelEnd = micros();

  // ── Cruise ─────────────────────────────────────────────────────────────────
  if (cruiseSteps > 0) {
    stepDelay = (unsigned long)(1000000.0 / cruiseSpeed);
    m->speedRPS = cruiseSpeed / m->stepsPerRev;
    for (int i = 0; i < cruiseSteps; i++) {
      if (limitTriggered(m)) {
        Serial.print("Limit hit during cruise at ");
        Serial.print(m->speedRPS, 3); Serial.println(" RPS — decelling to stop.");
        runLimitDecel(m, cruiseSpeed, dir);
        break;
      }
      digitalWrite(m->stepPin, HIGH);
      delayMicroseconds(stepDelay / 2);
      digitalWrite(m->stepPin, LOW);
      delayMicroseconds(stepDelay / 2);
      m->position += dir;
    }
  }
  cruiseEnd = micros();

  // ── Decel ──────────────────────────────────────────────────────────────────
  for (int i = 0; i < decelSteps; i++) {
    if (limitTriggered(m)) {
      Serial.print("Limit hit during decel at ");
      Serial.print(m->speedRPS, 3); Serial.println(" RPS — decelling to stop.");
      runLimitDecel(m, m->speedRPS * m->stepsPerRev, dir);
      break;
    }
    speed = sqrt(2.0 * decelRate * (decelSteps - i));
    if (speed < 1.0) speed = 1.0;
    stepDelay = (unsigned long)(1000000.0 / speed);
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(stepDelay / 2);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(stepDelay / 2);
    m->position += dir;
    m->speedRPS = speed / m->stepsPerRev;
  }
  m->speedRPS = 0;
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

  float commandedRevs = (float)(accelSteps + cruiseSteps + decelSteps) / m->stepsPerRev;

  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Move Complete ---");
  Serial.print("Commanded: "); Serial.print(commandedRevs, 3); Serial.println(" rev");
  Serial.print("Position: "); Serial.println(m->position);

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

} // anonymous namespace

// ── Public move functions ─────────────────────────────────────────────────────

namespace MotionProf {

void profileMove(MotorConfig* m,
                 float accelRevs, float cruiseRevs, float decelRevs,
                 float cruiseRPS) {
  STR3::setDir(m, accelRevs > 0);
  int8_t dir = (accelRevs > 0) ? 1 : -1;

  int aSteps = (int)(abs(accelRevs)  * m->stepsPerRev);
  int cSteps = (int)(abs(cruiseRevs) * m->stepsPerRev);
  int dSteps = (int)(abs(decelRevs)  * m->stepsPerRev);

  float cruiseSpeed = cruiseRPS * m->stepsPerRev;
  float accelRate   = (cruiseSpeed * cruiseSpeed) / (2.0 * aSteps);
  float decelRate   = (cruiseSpeed * cruiseSpeed) / (2.0 * dSteps);

  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Profile Move ---");
  Serial.print("Accel="); Serial.print(accelRevs);
  Serial.print(", Cruise="); Serial.print(cruiseRevs);
  Serial.print(", Decel="); Serial.print(decelRevs);
  Serial.print(" rev, RPS="); Serial.println(cruiseRPS);

  runTrapezoid(m, aSteps, cSteps, dSteps, cruiseSpeed, accelRate, decelRate, dir);
}

void trapezoidalMove(MotorConfig* m, float revolutions, float maxRPS, float totalTime) {
  int    totalSteps = abs(revolutions * m->stepsPerRev);
  STR3::setDir(m, revolutions > 0);
  int8_t dir = (revolutions > 0) ? 1 : -1;

  float maxSpeed = maxRPS * m->stepsPerRev;
  float tAccel   = totalTime - (totalSteps / maxSpeed);
  float tCruise  = totalTime - 2.0 * tAccel;

  if (tAccel <= 0 || tCruise < 0) {
    // Triangular — cannot reach maxRPS in the given time / distance
    float peak      = (2.0 * totalSteps) / totalTime;
    float tRamp     = totalTime / 2.0;
    float a         = peak / tRamp;
    int   halfSteps = totalSteps / 2;
    runTrapezoid(m, halfSteps, 0, totalSteps - halfSteps, peak, a, a, dir);
  } else {
    float a    = maxSpeed / tAccel;
    int aSteps = (int)(0.5 * a * tAccel * tAccel);
    int cSteps = (int)(maxSpeed * tCruise);
    int dSteps = totalSteps - aSteps - cSteps;
    runTrapezoid(m, aSteps, cSteps, dSteps, maxSpeed, a, a, dir);
  }
}

void rotate(MotorConfig* m, float revolutions, float rps) {
  int    total      = abs((int)(revolutions * m->stepsPerRev));
  int8_t dir        = (revolutions > 0) ? 1 : -1;
  unsigned long halfPeriod = (unsigned long)(500000.0 / (rps * m->stepsPerRev));

  STR3::setDir(m, revolutions > 0);
  m->speedRPS = rps;
  for (int i = 0; i < total; i++) {
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(halfPeriod);
    m->position += dir;
  }
  m->speedRPS = 0;
}

} // namespace MotionProf
