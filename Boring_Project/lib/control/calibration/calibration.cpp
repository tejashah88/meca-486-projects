// calibration.cpp
// Homing routines and position-based moves for limit-switched axes.

#include "calibration.h"
#include "../../driver/str3/str3.h"
#include "../display/display.h"
#include "../motion_prof/motion_prof.h"

// ── Private helpers ───────────────────────────────────────────────────────────

namespace {

// Step at constant velocity until sensorPin reads LOW (sensor triggered).
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

// Step at constant velocity until sensorPin reads HIGH (sensor released).
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

} // anonymous namespace

// ── Public calibration functions ──────────────────────────────────────────────

namespace Cal {

void homeAxis(MotorConfig* m, float slowRPS) {
  if (!m->hasLimits) { Serial.println("Cal::homeAxis: no limits configured."); return; }
  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Homing ---");

  if (digitalRead(m->limitHomePin) == LOW) {
    Serial.println("Already on home sensor — backing off until clear.");
    STR3::setDir(m, true);  // toward end = away from home
    creepUntilSensorClear(m, m->limitHomePin, 1, slowRPS);
  } else {
    STR3::setDir(m, false);  // toward home
    creepUntilSensor(m, m->limitHomePin, -1, slowRPS);
    Serial.println("Home sensor detected — backing off until clear.");
    STR3::setDir(m, true);   // away from home
    creepUntilSensorClear(m, m->limitHomePin, 1, slowRPS);
  }
  m->position = 0;
  Serial.println("Home set. Position = 0.");
  Display::updateMotor(m);
}

void findEnd(MotorConfig* m, float slowRPS) {
  if (!m->hasLimits) { Serial.println("Cal::findEnd: no limits configured."); return; }
  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Finding End ---");

  if (digitalRead(m->limitEndPin) == LOW) {
    Serial.println("Already at end.");
    m->endPos     = m->position;
    m->axisLength = m->endPos;
    Display::updateMotor(m);
    return;
  }
  STR3::setDir(m, true);  // toward end
  creepUntilSensor(m, m->limitEndPin, 1, slowRPS);
  m->endPos     = m->position;
  m->axisLength = m->endPos;
  Serial.print("End found at "); Serial.print(m->endPos);
  Serial.print(" steps ("); Serial.print((float)m->axisLength / m->stepsPerRev, 3);
  Serial.println(" revs)");
  Display::updateMotor(m);
}

void calibrateAxis(MotorConfig* m, float slowRPS) {
  if (!m->hasLimits) { Serial.println("Cal::calibrateAxis: no limits configured."); return; }
  homeAxis(m, slowRPS);
  findEnd(m, slowRPS);

  float stepRevs = (float)m->axisLength / m->stepsPerRev;
  Serial.print("--- Motor "); Serial.print(m->id); Serial.println(" Calibration Complete ---");
  Serial.print("Axis (steps): "); Serial.print(m->axisLength); Serial.print(" steps | ");
  Serial.print(stepRevs, 3); Serial.print(" revs");
  if (m->mmPerRev > 0.0f) {
    Serial.print(" | "); Serial.print(stepRevs * m->mmPerRev, 2); Serial.print(" mm");
  }
  Serial.println();
}

void moveToHome(MotorConfig* m, float cruiseRPS) {
  if (!m->hasLimits) { Serial.println("Cal::moveToHome: no limits configured."); return; }
  float totalRevs = (float)m->position / m->stepsPerRev;
  if (abs(totalRevs) < 0.01f) { Serial.println("Already at home."); return; }
  float ramp   = min(2.0f, abs(totalRevs) / 3.0f);
  float cruise = abs(totalRevs) - 2.0f * ramp;
  MotionProf::profileMove(m, -ramp, -cruise, -ramp, cruiseRPS);
}

void moveToEnd(MotorConfig* m, float cruiseRPS) {
  if (!m->hasLimits) { Serial.println("Cal::moveToEnd: no limits configured."); return; }
  float totalRevs = (float)(m->endPos - m->position) / m->stepsPerRev;
  if (abs(totalRevs) < 0.01f) { Serial.println("Already at end."); return; }
  float ramp   = min(2.0f, abs(totalRevs) / 3.0f);
  float cruise = abs(totalRevs) - 2.0f * ramp;
  MotionProf::profileMove(m, ramp, cruise, ramp, cruiseRPS);
}

} // namespace Cal
