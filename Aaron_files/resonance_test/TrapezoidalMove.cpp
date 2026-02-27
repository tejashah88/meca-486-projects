// TrapezoidalMove.cpp
// Unified trapezoidal motion profile — all functions take a MotorConfig*.

#include "TrapezoidalMove.h"

// ── Initialization ────────────────────────────────────────────────────────

void motorInit(MotorConfig* m,
               uint8_t        id,
               bool           hasLimits,
               int            dirPin,
               int            stepPin,
               int            buttonPin,
               int            stepsPerRev,
               LiquidCrystal* lcd,
               int            limitEndPin,
               int            limitHomePin) {
  m->id           = id;
  m->hasLimits    = hasLimits;
  m->dirPin       = dirPin;
  m->stepPin      = stepPin;
  m->buttonPin    = buttonPin;
  m->stepsPerRev  = stepsPerRev;
  m->lcd          = lcd;
  m->limitEndPin    = limitEndPin;
  m->limitHomePin   = limitHomePin;
  m->limitStopRevs  = 2.0f;  // max soft-stop distance (revs); actual = min(required by speed, this). 0 = instant stop
  m->position       = 0;
  m->endPos       = 0;
  m->axisLength   = 0;
  m->speedRPS     = 0.0;

  pinMode(dirPin,  OUTPUT);
  pinMode(stepPin, OUTPUT);
  if (buttonPin >= 0) pinMode(buttonPin, INPUT_PULLUP);

  if (hasLimits) {
    // INPUT_PULLUP ensures floating "not triggered" NPN output reads reliably HIGH
    if (limitEndPin  >= 0) pinMode(limitEndPin,  INPUT_PULLUP);
    if (limitHomePin >= 0) pinMode(limitHomePin, INPUT_PULLUP);
  }
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

// ── Limit checking ────────────────────────────────────────────────────────

// Direction-aware: only blocks movement toward the triggered limit.
// Always returns false when hasLimits = false.
static bool limitTriggered(MotorConfig* m) {
  if (!m->hasLimits) return false;
  bool movingTowardEnd = (digitalRead(m->dirPin) == LOW);
  if (movingTowardEnd) return digitalRead(m->limitEndPin)  == LOW;
  else                 return digitalRead(m->limitHomePin) == LOW;
}

// Decelerate from current speed to zero when limit hit. Uses move's decel rate to compute
// minimum distance; caps at limitStopRevs so we never exceed max soft-stop distance.
static void runLimitDecel(MotorConfig* m, float currentSpeedStepsPerSec, int8_t dir,
                          float decelRateStepsPerSecSq) {
  if (m->limitStopRevs <= 0) return;
  if (currentSpeedStepsPerSec < 1.0f) currentSpeedStepsPerSec = 1.0f;
  int maxStopSteps = (int)(m->limitStopRevs * m->stepsPerRev);
  if (maxStopSteps < 1) return;
  // v² = 2*a*d → d = v²/(2*a). Use move's decel rate so we know the motor can do it.
  int requiredSteps = (int)((currentSpeedStepsPerSec * currentSpeedStepsPerSec) / (2.0f * decelRateStepsPerSecSq));
  int stopSteps = requiredSteps;
  if (stopSteps > maxStopSteps) stopSteps = maxStopSteps;
  if (stopSteps < 1) stopSteps = 1;
  float decelRate = (currentSpeedStepsPerSec * currentSpeedStepsPerSec) / (2.0f * stopSteps);
  for (int j = 0; j < stopSteps; j++) {
    float speed = sqrt(2.0 * decelRate * (stopSteps - j));
    if (speed < 1.0f) speed = 1.0f;
    unsigned long stepDelay = (unsigned long)(1000000.0 / speed);
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

  // ── Accel ──
  startTime = micros();
  for (int i = 0; i < accelSteps; i++) {
    speed = sqrt(2.0 * accelRate * i);
    if (speed < 1.0) speed = 1.0;
    if (limitTriggered(m)) {
      Serial.println("Limit hit during accel — decelling to stop.");
      runLimitDecel(m, speed, dir, accelRate);
      break;
    }
    stepDelay = (unsigned long)(1000000.0 / speed);
    m->speedRPS = speed / m->stepsPerRev;
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(stepDelay / 2);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(stepDelay / 2);
    m->position += dir;
  }
  accelEnd = micros();

  // ── Cruise ──
  if (cruiseSteps > 0) {
    stepDelay = (unsigned long)(1000000.0 / cruiseSpeed);
    m->speedRPS = cruiseSpeed / m->stepsPerRev;
    for (int i = 0; i < cruiseSteps; i++) {
      if (limitTriggered(m)) {
        Serial.println("Limit hit during cruise — decelling to stop.");
        runLimitDecel(m, cruiseSpeed, dir, decelRate);
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
    speed = sqrt(2.0 * decelRate * (decelSteps - i));
    if (speed < 1.0) speed = 1.0;
    if (limitTriggered(m)) {
      Serial.println("Limit hit during decel — decelling to stop.");
      runLimitDecel(m, speed, dir, decelRate);
      break;
    }
    stepDelay = (unsigned long)(1000000.0 / speed);
    m->speedRPS = speed / m->stepsPerRev;
    digitalWrite(m->stepPin, HIGH);
    delayMicroseconds(stepDelay / 2);
    digitalWrite(m->stepPin, LOW);
    delayMicroseconds(stepDelay / 2);
    m->position += dir;
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

  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Move Complete ---");
  Serial.print("Steps: "); Serial.println(accelSteps + cruiseSteps + decelSteps);
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
  digitalWrite(m->dirPin, accelRevs > 0 ? LOW : HIGH);
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
  digitalWrite(m->dirPin, revolutions > 0 ? LOW : HIGH);
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

  digitalWrite(m->dirPin, revolutions > 0 ? LOW : HIGH);
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
    digitalWrite(m->dirPin, LOW);  // toward end = away from home
    creepUntilSensorClear(m, m->limitHomePin, 1, slowRPS);
  } else {
    digitalWrite(m->dirPin, HIGH);  // toward home
    creepUntilSensor(m, m->limitHomePin, -1, slowRPS);
    Serial.println("Home sensor detected — backing off until clear.");
    digitalWrite(m->dirPin, LOW);   // toward end = away from home
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
  digitalWrite(m->dirPin, LOW);  // LOW = toward end
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
  findEnd(m, slowRPS);
  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Calibration Complete ---");
  Serial.print("Axis length: "); Serial.print(m->axisLength);
  Serial.print(" steps ("); Serial.print((float)m->axisLength / m->stepsPerRev, 3);
  Serial.println(" revs)");
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
