// TrapezoidalMove.cpp
// Implementation of trapezoidal motion profile

#include "TrapezoidalMove.h"

void updateLCD() {
  static unsigned long lastUpdate = 0;
  if (micros() - lastUpdate < 100000) return; // max 10 Hz
  lastUpdate = micros();

  bool atHome = (digitalRead(SENSOR_PIN_2) == LOW);
  bool atEnd  = (digitalRead(SENSOR_PIN_1) == LOW);

  // Line 1: position — dtostrf avoids %f which Arduino snprintf doesn't support
  char posStr[8];
  dtostrf((float)motorPosition / STEPS_PER_REV, 6, 2, posStr); // e.g. "  1.23"
  char line[17];
  snprintf(line, sizeof(line), "Pos:%s rev  ", posStr); // always 16 chars
  lcd.setCursor(0, 0);
  lcd.print(line);

  // Line 2: limit status
  lcd.setCursor(0, 1);
  if      (atEnd && atHome) lcd.print("!!BOTH LIMITS!! ");
  else if (atEnd)           lcd.print("** END  LIMIT **");
  else if (atHome)          lcd.print("** HOME LIMIT **");
  else                      lcd.print("   Status: OK   ");
}

bool limitTriggered() {
  bool movingTowardEnd = (digitalRead(DIR_PIN) == LOW);
  if (movingTowardEnd) {
    return digitalRead(SENSOR_PIN_1) == LOW; // block only end limit
  } else {
    return digitalRead(SENSOR_PIN_2) == LOW; // block only home limit
  }
}

void profileMove(float accelRevs, float cruiseRevs, float decelRevs, float cruiseRPS) {
  digitalWrite(DIR_PIN, accelRevs > 0 ? LOW : HIGH);
  int8_t dir = (accelRevs > 0) ? 1 : -1;

  int accelSteps  = (int)(abs(accelRevs)  * STEPS_PER_REV);
  int cruiseSteps = (int)(abs(cruiseRevs) * STEPS_PER_REV);
  int decelSteps  = (int)(abs(decelRevs)  * STEPS_PER_REV);

  float cruiseSpeed = cruiseRPS * STEPS_PER_REV; // steps/sec

  // Derived accel/decel rates from distance and velocity
  // Using v^2 = 2*a*d → a = v^2 / (2*d)
  float accel = (cruiseSpeed * cruiseSpeed) / (2.0 * accelSteps);
  float decel = (cruiseSpeed * cruiseSpeed) / (2.0 * decelSteps);

  // Expected times for each phase
  float t_accel_expected  = cruiseSpeed / accel;           // t = v/a
  float t_cruise_expected = (float)cruiseSteps / cruiseSpeed; // t = d/v
  float t_decel_expected  = cruiseSpeed / decel;

  Serial.println("--- Profile Move ---");
  Serial.print("Accel revs="); Serial.print(accelRevs);
  Serial.print(", Cruise revs="); Serial.print(cruiseRevs);
  Serial.print(", Decel revs="); Serial.println(decelRevs);
  Serial.print("Cruise RPS="); Serial.println(cruiseRPS);

  float currentSpeed;
  unsigned long stepDelayMicros;
  unsigned long startTime, accelTime, cruiseTime, decelTime;

  // --- Acceleration phase ---
  startTime = micros();
  for (int i = 0; i < accelSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during accel!"); break; }
    currentSpeed = sqrt(2.0 * accel * i);
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    currentSpeedRPS = currentSpeed / STEPS_PER_REV;

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  accelTime = micros();

  // --- Cruise phase ---
  currentSpeedRPS = cruiseSpeed / STEPS_PER_REV;
  stepDelayMicros = (unsigned long)(1000000.0 / cruiseSpeed);
  for (int i = 0; i < cruiseSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during cruise!"); break; }
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  cruiseTime = micros();

  // --- Deceleration phase ---
  for (int i = 0; i < decelSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during decel!"); break; }
    int stepsRemaining = decelSteps - i;
    currentSpeed = sqrt(2.0 * decel * stepsRemaining);
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    currentSpeedRPS = currentSpeed / STEPS_PER_REV;

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  currentSpeedRPS = 0;
  decelTime = micros();

  // --- Report ---
  float t_accel_actual  = (accelTime  - startTime)  / 1000000.0;
  float t_cruise_actual = (cruiseTime - accelTime)   / 1000000.0;
  float t_decel_actual  = (decelTime  - cruiseTime)  / 1000000.0;
  float t_total_actual  = (decelTime  - startTime)   / 1000000.0;
  float t_total_expected = t_accel_expected + t_cruise_expected + t_decel_expected;
  float error = t_total_actual - t_total_expected;
  float errorPercent = (error / t_total_expected) * 100.0;

  Serial.println("--- Move Complete ---");
  Serial.print("Total Steps: "); Serial.println(accelSteps + cruiseSteps + decelSteps);
  Serial.print("Accel:  Expected="); Serial.print(t_accel_expected, 3);
  Serial.print("s, Actual="); Serial.print(t_accel_actual, 3); Serial.println("s");
  Serial.print("Cruise: Expected="); Serial.print(t_cruise_expected, 3);
  Serial.print("s, Actual="); Serial.print(t_cruise_actual, 3); Serial.println("s");
  Serial.print("Decel:  Expected="); Serial.print(t_decel_expected, 3);
  Serial.print("s, Actual="); Serial.print(t_decel_actual, 3); Serial.println("s");
  Serial.print("Total:  Expected="); Serial.print(t_total_expected, 3);
  Serial.print("s, Actual="); Serial.print(t_total_actual, 3);
  Serial.print("s, Error="); Serial.print(error, 3);
  Serial.print("s ("); Serial.print(errorPercent, 2); Serial.println("%)");
  updateLCD();
}

// trapezoidalMove: move a given number of revolutions in a fixed total time,
// trying to reach maxRPS at cruise speed.
//
// The motion profile looks like one of two shapes depending on whether
// there is enough distance to reach maxRPS:
//
//   Trapezoidal (can reach maxRPS):       Triangular (too short, can't reach maxRPS):
//
//   speed                                  speed
//     |     ___________                      |       /\
//     |    /           \                     |      /  \
//     |   /             \                    |     /    \
//     |  /               \                   |    /      \
//     |_/                 \__                |___/        \___
//        accel  cruise  decel   time              accel decel  time
//
// The key constraint is: total distance = area under the speed-time curve.
// For trapezoidal: D = V_max * t_cruise + V_max * t_accel
//   Rearranging for t_accel: t_accel = T - D/V_max
//   Then: t_cruise = T - 2*t_accel
//
// If t_accel <= 0 or t_cruise < 0, the distance is too short to reach
// maxRPS in the given time, so we fall back to a triangular profile where
// the motor ramps up to a lower peak speed and immediately decelerates.
//   Peak speed for triangle: V_peak = 2*D/T  (area of triangle = D)

void trapezoidalMove(float revolutions, float maxRPS, float totalTime) {
  int totalSteps = abs(revolutions * STEPS_PER_REV);
  digitalWrite(DIR_PIN, revolutions > 0 ? LOW : HIGH);

  // Convert the cruise speed cap from RPS to steps/sec
  float maxSpeed = maxRPS * STEPS_PER_REV;

  // Solve for how long the accel and cruise phases need to be.
  // Derived from: D = V*(T - t_accel)  →  t_accel = T - D/V
  float t_accel  = totalTime - (totalSteps / maxSpeed);
  float t_cruise = totalTime - 2.0 * t_accel;

  Serial.print("t_accel="); Serial.print(t_accel, 3);
  Serial.print("s, t_cruise="); Serial.print(t_cruise, 3); Serial.println("s");

  if (t_accel <= 0 || t_cruise < 0) {
    // Distance too short to reach maxRPS — use a triangular profile instead.
    // Peak speed is whatever fills the triangle: V_peak = 2*D/T
    float peakSpeed = (2.0 * totalSteps) / totalTime;
    float t_ramp    = totalTime / 2.0;           // equal accel and decel time
    float accel     = peakSpeed / t_ramp;        // a = V/t

    executeTriangular(totalSteps, peakSpeed, accel, t_ramp);
  } else {
    // Enough distance — use a full trapezoidal profile.
    // Acceleration rate: a = V/t  (ramp linearly from 0 to maxSpeed)
    float accel = maxSpeed / t_accel;

    // Step counts for each phase using kinematics: d = ½*a*t²  and  d = V*t
    int accelSteps  = (int)(0.5 * accel * t_accel * t_accel);
    int cruiseSteps = (int)(maxSpeed * t_cruise);
    int decelSteps  = totalSteps - accelSteps - cruiseSteps; // remainder

    executeTrapezoidal(accelSteps, cruiseSteps, decelSteps, maxSpeed, accel, t_accel, t_cruise);
  }
}

void executeTrapezoidal(int accelSteps, int cruiseSteps, int decelSteps, float maxSpeed, float accel, float t_accel_expected, float t_cruise_expected) {
  int8_t dir = (digitalRead(DIR_PIN) == LOW) ? 1 : -1;
  unsigned long stepDelayMicros;
  float currentSpeed;
  unsigned long startTime, accelTime, cruiseTime, decelTime;
  int totalSteps = accelSteps + cruiseSteps + decelSteps;
  
  // Acceleration phase
  startTime = micros();
  for (int i = 0; i < accelSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during accel!"); break; }
    float t = sqrt((2.0 * i) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    
    currentSpeedRPS = currentSpeed / STEPS_PER_REV;
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  accelTime = micros();
  
  // Cruise phase
  currentSpeedRPS = maxSpeed / STEPS_PER_REV;
  stepDelayMicros = (unsigned long)(1000000.0 / maxSpeed);
  for (int i = 0; i < cruiseSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during cruise!"); break; }
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  cruiseTime = micros();
  
  // Deceleration phase
  for (int i = 0; i < decelSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during decel!"); break; }
    int stepsRemaining = decelSteps - i;
    float t = sqrt((2.0 * stepsRemaining) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    currentSpeedRPS = currentSpeed / STEPS_PER_REV;
    
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  currentSpeedRPS = 0;
  decelTime = micros();
  
  // Calculate actual times
  float t_accel_actual = (accelTime - startTime) / 1000000.0;
  float t_cruise_actual = (cruiseTime - accelTime) / 1000000.0;
  float t_decel_actual = (decelTime - cruiseTime) / 1000000.0;
  float t_total_actual = (decelTime - startTime) / 1000000.0;
  
  Serial.println("--- Move Complete: Trapezoidal ---");
  Serial.print("Total Steps: ");
  Serial.println(totalSteps);
  
  Serial.print("Accel:  Expected=");
  Serial.print(t_accel_expected, 3);
  Serial.print("s, Actual=");
  Serial.print(t_accel_actual, 3);
  Serial.println("s");
  
  Serial.print("Cruise: Expected=");
  Serial.print(t_cruise_expected, 3);
  Serial.print("s, Actual=");
  Serial.print(t_cruise_actual, 3);
  Serial.println("s");
  
  Serial.print("Decel:  Expected=");
  Serial.print(t_accel_expected, 3);
  Serial.print("s, Actual=");
  Serial.print(t_decel_actual, 3);
  Serial.println("s");
  
  float t_total_expected = t_accel_expected * 2.0 + t_cruise_expected;
  float error = t_total_actual - t_total_expected;
  float errorPercent = (error / t_total_expected) * 100.0;
  
  Serial.print("Total:  Expected=");
  Serial.print(t_total_expected, 3);
  Serial.print("s, Actual=");
  Serial.print(t_total_actual, 3);
  Serial.print("s, Error=");
  Serial.print(error, 3);
  Serial.print("s (");
  Serial.print(errorPercent, 2);
  Serial.println("%)");
}

void executeTriangular(int totalSteps, float peakSpeed, float accel, float t_ramp) {
  int8_t dir = (digitalRead(DIR_PIN) == LOW) ? 1 : -1;
  int halfSteps = totalSteps / 2;
  unsigned long stepDelayMicros;
  float currentSpeed;
  unsigned long startTime, accelTime, decelTime;
  
  // Acceleration phase
  startTime = micros();
  for (int i = 0; i < halfSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during accel!"); break; }
    float t = sqrt((2.0 * i) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    
    currentSpeedRPS = currentSpeed / STEPS_PER_REV;
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  accelTime = micros();
  
  // Deceleration phase
  int remainingSteps = totalSteps - halfSteps;
  for (int i = 0; i < remainingSteps; i++) {
    if (limitTriggered()) { Serial.println("Limit hit during decel!"); break; }
    int stepsRemaining = remainingSteps - i;
    float t = sqrt((2.0 * stepsRemaining) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    currentSpeedRPS = currentSpeed / STEPS_PER_REV;
    
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
    motorPosition += dir;
  }
  currentSpeedRPS = 0;
  decelTime = micros();
  
  // Calculate actual times
  float t_accel_actual = (accelTime - startTime) / 1000000.0;
  float t_decel_actual = (decelTime - accelTime) / 1000000.0;
  float t_total_actual = (decelTime - startTime) / 1000000.0;
  
  Serial.println("--- Move Complete: Triangular ---");
  Serial.print("Total Steps: ");
  Serial.println(totalSteps);
  
  Serial.print("Accel:  Expected=");
  Serial.print(t_ramp, 3);
  Serial.print("s, Actual=");
  Serial.print(t_accel_actual, 3);
  Serial.println("s");
  
  Serial.print("Cruise: Expected=0.000s, Actual=0.000s");
  Serial.println();
  
  Serial.print("Decel:  Expected=");
  Serial.print(t_ramp, 3);
  Serial.print("s, Actual=");
  Serial.print(t_decel_actual, 3);
  Serial.println("s");
  
  float t_total_expected = t_ramp * 2.0;
  float error = t_total_actual - t_total_expected;
  float errorPercent = (error / t_total_expected) * 100.0;
  
  Serial.print("Total:  Expected=");
  Serial.print(t_total_expected, 3);
  Serial.print("s, Actual=");
  Serial.print(t_total_actual, 3);
  Serial.print("s, Error=");
  Serial.print(error, 3);
  Serial.print("s (");
  Serial.print(errorPercent, 2);
  Serial.println("%)");
}

void homeAxis(float slowRPS) {
  Serial.println("--- Homing ---");
  if (digitalRead(SENSOR_PIN_2) == LOW) {
    Serial.println("Already at home.");
    motorPosition = 0;
    return;
  }
  digitalWrite(DIR_PIN, HIGH); // negative direction = toward home
  unsigned long stepDelay = (unsigned long)(500000.0 / (slowRPS * STEPS_PER_REV));
  while (digitalRead(SENSOR_PIN_2) == HIGH) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
    motorPosition--;
  }
  motorPosition = 0;
  Serial.println("Home found. Position = 0.");
  updateLCD();
}

void findEnd(float slowRPS) {
  Serial.println("--- Finding End ---");
  if (digitalRead(SENSOR_PIN_1) == LOW) {
    Serial.println("Already at end.");
    endPosition = motorPosition;
    axisLength   = endPosition;
    return;
  }
  digitalWrite(DIR_PIN, LOW); // positive direction = toward end
  unsigned long stepDelay = (unsigned long)(500000.0 / (slowRPS * STEPS_PER_REV));
  while (digitalRead(SENSOR_PIN_1) == HIGH) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
    motorPosition++;
  }
  endPosition = motorPosition;
  axisLength  = endPosition;
  Serial.print("End found. endPosition="); Serial.print(endPosition);
  Serial.print(" steps, axisLength="); Serial.print(axisLength);
  Serial.println(" steps");
  updateLCD();
}

void calibrateAxis(float slowRPS) {
  homeAxis(slowRPS);
  findEnd(slowRPS);
  Serial.println("--- Calibration Complete ---");
  Serial.print("Axis length: "); Serial.print(axisLength);
  Serial.print(" steps ("); Serial.print((float)axisLength / STEPS_PER_REV, 3);
  Serial.println(" revs)");
}

void moveToHome(float cruiseRPS) {
  float totalRevs = (float)motorPosition / STEPS_PER_REV;
  if (abs(totalRevs) < 0.01) {
    Serial.println("Already at home.");
    return;
  }
  float rampRevs  = min(2.0f, abs(totalRevs) / 3.0f);
  float cruiseRevs = abs(totalRevs) - 2.0 * rampRevs;
  Serial.println("--- Move To Home ---");
  profileMove(-rampRevs, -cruiseRevs, -rampRevs, cruiseRPS);
}

void moveToEnd(float cruiseRPS) {
  float totalRevs = (float)(endPosition - motorPosition) / STEPS_PER_REV;
  if (abs(totalRevs) < 0.01) {
    Serial.println("Already at end.");
    return;
  }
  float rampRevs  = min(2.0f, abs(totalRevs) / 3.0f);
  float cruiseRevs = abs(totalRevs) - 2.0 * rampRevs;
  Serial.println("--- Move To End ---");
  profileMove(rampRevs, cruiseRevs, rampRevs, cruiseRPS);
}

void rotate(float revolutions, float rps) {
  int totalSteps = revolutions * STEPS_PER_REV;
  int stepDelay = (int)(500000.0 / (rps * STEPS_PER_REV)); // microseconds (half period)
  
  digitalWrite(DIR_PIN, revolutions > 0 ? HIGH : LOW);
  totalSteps = abs(totalSteps);
  
  for (int i = 0; i < totalSteps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
  }
}
