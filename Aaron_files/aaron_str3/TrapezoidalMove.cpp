// TrapezoidalMove.cpp
// Unified trapezoidal motion profile — all functions take a MotorConfig*.

#include "TrapezoidalMove.h"

// ── Initialization ────────────────────────────────────────────────────────

void motorInit(MotorConfig* m,
               uint8_t        id,
               bool           hasLimits,
               int            dirPin,
               int            stepPin,
               int            stepsPerRev,
               bool           invertDir,
               LiquidCrystal* lcd,
               int            limitEndPin,
               int            limitHomePin,
               float          mmPerRev,
               int            tachPin,
               uint8_t        tachPulsesPerRev,
               float          maxRPS) {
  m->id           = id;
  m->hasLimits    = hasLimits;
  m->invertDir    = invertDir;
  m->dirPin       = dirPin;
  m->stepPin      = stepPin;
  m->stepsPerRev  = stepsPerRev;
  m->lcd          = lcd;
  m->limitEndPin       = limitEndPin;
  m->limitHomePin      = limitHomePin;
  m->mmPerRev          = mmPerRev;
  m->tachPin           = tachPin;
  m->tachPulsesPerRev  = tachPulsesPerRev;
  m->tachCount         = 0;
  m->maxRPS            = maxRPS;
  m->limitStopRevs     = 2.0f;  // max stop distance at maxRPS; lower speeds stop proportionally shorter
  m->position       = 0;
  m->endPos         = 0;
  m->axisLength     = 0;
  m->speedRPS       = 0.0;
  m->limitEndFlag   = false;
  m->limitHomeFlag  = false;

  pinMode(dirPin,  OUTPUT);
  pinMode(stepPin, OUTPUT);

  if (hasLimits) {
    // INPUT_PULLUP ensures floating "not triggered" NPN output reads reliably HIGH
    if (limitEndPin  >= 0) pinMode(limitEndPin,  INPUT_PULLUP);
    if (limitHomePin >= 0) pinMode(limitHomePin, INPUT_PULLUP);
  }
}

// ── Direction helper ──────────────────────────────────────────────────────
// forward = true means positive / toward-end direction in logical space.
// invertDir flips the physical pin level to match actual wiring.
static inline void setDir(MotorConfig* m, bool forward) {
  digitalWrite(m->dirPin, (forward ^ m->invertDir) ? LOW : HIGH);
}

// ── LCD ───────────────────────────────────────────────────────────────────

void updateLCD(MotorConfig* m) {
  if (!m->lcd) return;

  static unsigned long lastUpdate = 0;
  if (micros() - lastUpdate < 100000) return;  // max 10 Hz
  lastUpdate = micros();

  char posStr[8];
  dtostrf((float)m->position / m->stepsPerRev, 6, 2, posStr);
  char line[17];
  snprintf(line, sizeof(line), "M%d Pos:%s rev", m->id, posStr);
  m->lcd->setCursor(0, 0);
  m->lcd->print(line);

  m->lcd->setCursor(0, 1);
  if (m->hasLimits) {
    bool atEnd  = (digitalRead(m->limitEndPin)  == LOW);
    bool atHome = (digitalRead(m->limitHomePin) == LOW);
    if      (atEnd && atHome) m->lcd->print("!!BOTH LIMITS!! ");
    else if (atEnd)           m->lcd->print("** END  LIMIT **");
    else if (atHome)          m->lcd->print("** HOME LIMIT **");
    else                      m->lcd->print("   Status: OK   ");
  } else {
    m->lcd->print("  Motor Only    ");
  }
}

// ── Interrupt-driven limit switch ISRs ────────────────────────────────────

#define MAX_LIMIT_MOTORS 4
static MotorConfig* _endMotors[MAX_LIMIT_MOTORS]  = {nullptr, nullptr, nullptr, nullptr};
static MotorConfig* _homeMotors[MAX_LIMIT_MOTORS] = {nullptr, nullptr, nullptr, nullptr};

static void endISR0()  { _endMotors[0]->limitEndFlag   = true; }
static void homeISR0() { _homeMotors[0]->limitHomeFlag  = true; }
static void endISR1()  { _endMotors[1]->limitEndFlag   = true; }
static void homeISR1() { _homeMotors[1]->limitHomeFlag  = true; }
static void endISR2()  { _endMotors[2]->limitEndFlag   = true; }
static void homeISR2() { _homeMotors[2]->limitHomeFlag  = true; }
static void endISR3()  { _endMotors[3]->limitEndFlag   = true; }
static void homeISR3() { _homeMotors[3]->limitHomeFlag  = true; }

typedef void (*IsrFunc)();
static const IsrFunc endISRs[]  = {endISR0,  endISR1,  endISR2,  endISR3};
static const IsrFunc homeISRs[] = {homeISR0, homeISR1, homeISR2, homeISR3};

void attachLimitInterrupts(MotorConfig* m) {
  if (!m->hasLimits) { Serial.println("attachLimitInterrupts: no limits configured."); return; }

  int slot = -1;
  for (int i = 0; i < MAX_LIMIT_MOTORS; i++) {
    if (_endMotors[i] == nullptr) { slot = i; break; }
  }
  if (slot < 0) { Serial.println("attachLimitInterrupts: no free ISR slots."); return; }

  _endMotors[slot]  = m;
  _homeMotors[slot] = m;

  if (m->limitEndPin  >= 0) attachInterrupt(digitalPinToInterrupt(m->limitEndPin),  endISRs[slot],  FALLING);
  if (m->limitHomePin >= 0) attachInterrupt(digitalPinToInterrupt(m->limitHomePin), homeISRs[slot], FALLING);

  m->limitEndFlag  = false;
  m->limitHomeFlag = false;
  Serial.print("Limit ISRs attached for motor "); Serial.print(m->id);
  Serial.print(" (slot "); Serial.print(slot); Serial.println(")");
}

// ── Tachometer ISRs ───────────────────────────────────────────────────────

static MotorConfig* _tachMotors[MAX_LIMIT_MOTORS] = {nullptr, nullptr, nullptr, nullptr};

static void tachISR0() { _tachMotors[0]->tachCount++; }
static void tachISR1() { _tachMotors[1]->tachCount++; }
static void tachISR2() { _tachMotors[2]->tachCount++; }
static void tachISR3() { _tachMotors[3]->tachCount++; }

static const IsrFunc tachISRs[] = {tachISR0, tachISR1, tachISR2, tachISR3};

void attachTachInterrupt(MotorConfig* m) {
  if (m->tachPin < 0) { Serial.println("attachTachInterrupt: no tach pin configured."); return; }

  int slot = -1;
  for (int i = 0; i < MAX_LIMIT_MOTORS; i++) {
    if (_tachMotors[i] == nullptr) { slot = i; break; }
  }
  if (slot < 0) { Serial.println("attachTachInterrupt: no free ISR slots."); return; }

  _tachMotors[slot] = m;
  m->tachCount = 0;
  pinMode(m->tachPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(m->tachPin), tachISRs[slot], FALLING);
  Serial.print("Tach ISR attached for motor "); Serial.print(m->id);
  Serial.print(" (slot "); Serial.print(slot); Serial.println(")");
}

void resetTach(MotorConfig* m) {
  noInterrupts();
  m->tachCount = 0;
  interrupts();
}

float getTachRevolutions(MotorConfig* m) {
  noInterrupts();
  uint32_t count = m->tachCount;
  interrupts();
  return count / (float)m->tachPulsesPerRev;
}

float getTachRPS(MotorConfig* m, uint16_t sampleMs) {
  noInterrupts();
  uint32_t c0 = m->tachCount;
  interrupts();
  unsigned long t0 = micros();
  delay(sampleMs);
  noInterrupts();
  uint32_t c1 = m->tachCount;
  interrupts();
  float elapsed = (micros() - t0) / 1e6f;
  return ((c1 - c0) / (float)m->tachPulsesPerRev) / elapsed;
}

// ── Limit checking ────────────────────────────────────────────────────────

// Direction-aware: only blocks movement toward the triggered limit.
// Uses ISR-set flags instead of polling digitalRead each step.
// Always returns false when hasLimits = false.
static bool limitTriggered(MotorConfig* m) {
  if (!m->hasLimits) return false;
  bool movingTowardEnd = (digitalRead(m->dirPin) == LOW) ^ m->invertDir;
  if (movingTowardEnd) return m->limitEndFlag;
  else                 return m->limitHomeFlag;
}

// Fixed decel rate: a = (maxRPS * stepsPerRev)² / (2 * limitStopRevs * stepsPerRev)
// This is the rate that stops the motor from maxRPS in exactly limitStopRevs revolutions.
// At the current (lower) speed, stop distance = v²/(2a) ≤ limitStopRevs revs.
// limitStopRevs = 0 or maxRPS = 0 → instant stop.
static void runLimitDecel(MotorConfig* m, float currentSpeedStepsPerSec, int8_t dir) {
  if (m->limitStopRevs <= 0.0f || m->maxRPS <= 0.0f) return;
  if (currentSpeedStepsPerSec < 1.0f) currentSpeedStepsPerSec = 1.0f;

  float maxSpeedSteps = m->maxRPS * m->stepsPerRev;
  int   maxStopSteps  = (int)(m->limitStopRevs * m->stepsPerRev);
  // Fixed decel rate based on max conditions: a = vmax² / (2 * dmax)
  float decelRate = (maxSpeedSteps * maxSpeedSteps) / (2.0f * maxStopSteps);
  // Actual stop steps at current speed: d = v² / (2a)
  int stopSteps = (int)(currentSpeedStepsPerSec * currentSpeedStepsPerSec / (2.0f * decelRate));
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

// ── Core step executor ────────────────────────────────────────────────────
// 3-phase move: accel → cruise → decel.
// Speed ramp: v = sqrt(2 * a * distance)

static void runTrapezoid(MotorConfig* m,
                         int accelSteps, int cruiseSteps, int decelSteps,
                         float cruiseSpeed, float accelRate, float decelRate,
                         int8_t dir) {
  unsigned long stepDelay;
  float speed;
  unsigned long startTime, accelEnd, cruiseEnd, decelEnd;

  // Pre-seed the flag for the limit we're moving toward in case the switch
  // is already held LOW (no FALLING edge will fire, so the ISR won't set it).
  if (m->hasLimits) {
    if (dir > 0) m->limitEndFlag  = (m->limitEndPin  >= 0 && digitalRead(m->limitEndPin)  == LOW);
    else         m->limitHomeFlag = (m->limitHomePin >= 0 && digitalRead(m->limitHomePin) == LOW);
  }

  resetTach(m);

  // ── Accel ──
  startTime = micros();
  for (int i = 0; i < accelSteps; i++) {
    // Check limit first — m->speedRPS reflects the last completed step's velocity.
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
    m->speedRPS = speed / m->stepsPerRev;  // updated after step completes
  }
  accelEnd = micros();

  // ── Cruise ──
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

  // ── Decel ──
  for (int i = 0; i < decelSteps; i++) {
    // Check limit first — m->speedRPS reflects the last completed step's velocity.
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
    m->speedRPS = speed / m->stepsPerRev;  // updated after step completes
  }
  m->speedRPS = 0;
  decelEnd = micros();

  // ── Serial timing report ──
  float tAccel  = (accelEnd  - startTime) / 1e6;
  float tCruise = (cruiseEnd - accelEnd)  / 1e6;
  float tDecel  = (decelEnd  - cruiseEnd) / 1e6;
  float tTotal  = (decelEnd  - startTime) / 1e6;

  float tAccelExp  = (accelSteps  > 0) ? cruiseSpeed / accelRate : 0;
  float tCruiseExp = (cruiseSteps > 0) ? (float)cruiseSteps / cruiseSpeed : 0;
  float tDecelExp  = (decelSteps  > 0) ? cruiseSpeed / decelRate : 0;
  float tTotalExp  = tAccelExp + tCruiseExp + tDecelExp;

  float tachRevs        = getTachRevolutions(m);
  float commandedRevs   = (float)(accelSteps + cruiseSteps + decelSteps) / m->stepsPerRev;
  float tachErr         = tachRevs - commandedRevs;

  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Move Complete ---");
  Serial.print("Commanded: "); Serial.print(commandedRevs, 3); Serial.print(" rev | ");
  Serial.print("Tach: ");      Serial.print(tachRevs,      3); Serial.print(" rev | ");
  Serial.print("Error: ");     Serial.print(tachErr,        3); Serial.println(" rev");
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

  updateLCD(m);
}

// ── Public move functions ─────────────────────────────────────────────────

void profileMove(MotorConfig* m,
                 float accelRevs, float cruiseRevs, float decelRevs,
                 float cruiseRPS) {
  setDir(m, accelRevs > 0);
  int8_t dir = (accelRevs > 0) ? 1 : -1;

  int aSteps = (int)(abs(accelRevs)  * m->stepsPerRev);
  int cSteps = (int)(abs(cruiseRevs) * m->stepsPerRev);
  int dSteps = (int)(abs(decelRevs)  * m->stepsPerRev);

  float cruiseSpeed = cruiseRPS * m->stepsPerRev;
  // a = v²/(2d)
  float accelRate = (cruiseSpeed * cruiseSpeed) / (2.0 * aSteps);
  float decelRate = (cruiseSpeed * cruiseSpeed) / (2.0 * dSteps);

  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Profile Move ---");
  Serial.print("Accel="); Serial.print(accelRevs);
  Serial.print(", Cruise="); Serial.print(cruiseRevs);
  Serial.print(", Decel="); Serial.print(decelRevs);
  Serial.print(" rev, RPS="); Serial.println(cruiseRPS);

  runTrapezoid(m, aSteps, cSteps, dSteps, cruiseSpeed, accelRate, decelRate, dir);
}

void trapezoidalMove(MotorConfig* m, float revolutions, float maxRPS, float totalTime) {
  int totalSteps = abs(revolutions * m->stepsPerRev);
  setDir(m, revolutions > 0);
  int8_t dir = (revolutions > 0) ? 1 : -1;

  float maxSpeed = maxRPS * m->stepsPerRev;
  float tAccel   = totalTime - (totalSteps / maxSpeed);
  float tCruise  = totalTime - 2.0 * tAccel;

  if (tAccel <= 0 || tCruise < 0) {
    // Triangular — can't reach maxRPS in the given time/distance
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

// ── Constant velocity ─────────────────────────────────────────────────────

void rotate(MotorConfig* m, float revolutions, float rps) {
  int    total      = abs((int)(revolutions * m->stepsPerRev));
  int8_t dir        = (revolutions > 0) ? 1 : -1;
  unsigned long halfPeriod = (unsigned long)(500000.0 / (rps * m->stepsPerRev));

  setDir(m, revolutions > 0);
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

// ── Homing & calibration ──────────────────────────────────────────────────

// Creep until sensor triggers (reads LOW). Steps in direction dir.
static void creepUntilSensor(MotorConfig* m, int sensorPin, int8_t dir, float rps) {
  unsigned long halfPeriod = (unsigned long)(500000.0 / (rps * m->stepsPerRev));
  while (digitalRead(sensorPin) == HIGH) {
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(halfPeriod);
    m->position += dir;
  }
}

// Creep until sensor releases (reads HIGH). Steps in direction dir.
static void creepUntilSensorClear(MotorConfig* m, int sensorPin, int8_t dir, float rps) {
  unsigned long halfPeriod = (unsigned long)(500000.0 / (rps * m->stepsPerRev));
  while (digitalRead(sensorPin) == LOW) {
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(halfPeriod);
    m->position += dir;
  }
}

void homeAxis(MotorConfig* m, float slowRPS) {
  if (!m->hasLimits) { Serial.println("homeAxis: no limits configured."); return; }
  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Homing ---");
  if (digitalRead(m->limitHomePin) == LOW) {
    Serial.println("Already on home sensor — backing off until clear.");
    setDir(m, true);   // toward end = away from home
    creepUntilSensorClear(m, m->limitHomePin, 1, slowRPS);
  } else {
    setDir(m, false);  // toward home
    creepUntilSensor(m, m->limitHomePin, -1, slowRPS);
    Serial.println("Home sensor detected — backing off until clear.");
    setDir(m, true);   // toward end = away from home
    creepUntilSensorClear(m, m->limitHomePin, 1, slowRPS);
  }
  m->position = 0;  // define home as "just clear" of the sensor
  Serial.println("Home set. Position = 0.");
  updateLCD(m);
}

void findEnd(MotorConfig* m, float slowRPS) {
  if (!m->hasLimits) { Serial.println("findEnd: no limits configured."); return; }
  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Finding End ---");
  if (digitalRead(m->limitEndPin) == LOW) {
    Serial.println("Already at end.");
    m->endPos     = m->position;
    m->axisLength = m->endPos;
    updateLCD(m);
    return;
  }
  setDir(m, true);  // toward end
  creepUntilSensor(m, m->limitEndPin, 1, slowRPS);
  m->endPos     = m->position;
  m->axisLength = m->endPos;
  Serial.print("End found at "); Serial.print(m->endPos);
  Serial.print(" steps ("); Serial.print((float)m->axisLength / m->stepsPerRev, 3);
  Serial.println(" revs)");
  updateLCD(m);
}

void calibrateAxis(MotorConfig* m, float slowRPS) {
  if (!m->hasLimits) { Serial.println("calibrateAxis: no limits configured."); return; }
  homeAxis(m, slowRPS);
  resetTach(m);   // reset after homing so tach measures only home→end travel
  findEnd(m, slowRPS);
  float tachRevs = getTachRevolutions(m);
  float stepRevs = (float)m->axisLength / m->stepsPerRev;
  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Calibration Complete ---");
  Serial.print("Axis (steps): "); Serial.print(m->axisLength); Serial.print(" steps | ");
  Serial.print(stepRevs, 3); Serial.print(" revs");
  if (m->mmPerRev > 0.0f) {
    Serial.print(" | "); Serial.print(stepRevs * m->mmPerRev, 2); Serial.print(" mm");
  }
  Serial.println();
  if (m->tachPin >= 0) {
    Serial.print("Axis (tach):  "); Serial.print(tachRevs, 3); Serial.print(" revs");
    if (m->mmPerRev > 0.0f) {
      Serial.print(" | "); Serial.print(tachRevs * m->mmPerRev, 2); Serial.print(" mm");
    }
    Serial.println();
  }
}

// ── Position-based moves ──────────────────────────────────────────────────

void moveToHome(MotorConfig* m, float cruiseRPS) {
  if (!m->hasLimits) { Serial.println("moveToHome: no limits configured."); return; }
  float totalRevs = (float)m->position / m->stepsPerRev;
  if (abs(totalRevs) < 0.01) { Serial.println("Already at home."); return; }
  float ramp   = min(2.0f, abs(totalRevs) / 3.0f);
  float cruise = abs(totalRevs) - 2.0 * ramp;
  profileMove(m, -ramp, -cruise, -ramp, cruiseRPS);
}

void moveToEnd(MotorConfig* m, float cruiseRPS) {
  if (!m->hasLimits) { Serial.println("moveToEnd: no limits configured."); return; }
  float totalRevs = (float)(m->endPos - m->position) / m->stepsPerRev;
  if (abs(totalRevs) < 0.01) { Serial.println("Already at end."); return; }
  float ramp   = min(2.0f, abs(totalRevs) / 3.0f);
  float cruise = abs(totalRevs) - 2.0 * ramp;
  profileMove(m, ramp, cruise, ramp, cruiseRPS);
}
