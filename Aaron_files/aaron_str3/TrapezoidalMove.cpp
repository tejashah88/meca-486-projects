// TrapezoidalMove.cpp
// Implementation of trapezoidal motion profile

#include "TrapezoidalMove.h"

void trapezoidalMove(float revolutions, float maxRPM, float totalTime) {
  int totalSteps = abs(revolutions * STEPS_PER_REV);
  digitalWrite(DIR_PIN, revolutions > 0 ? LOW : HIGH); // Negative revs = CCW (HIGH)
  
  // Convert RPM to steps/sec
  float maxSpeed = (maxRPM * STEPS_PER_REV) / 60.0; // steps/sec
  
  // Calculate if we can reach target velocity
  // For trapezoidal: D = V*(T - t_accel), where t_accel = (V*T - D)/V
  float t_accel = totalTime - (totalSteps / maxSpeed);
  float t_cruise = totalTime - 2.0 * t_accel;
  
  Serial.print("t_accel=");
  Serial.print(t_accel, 3);
  Serial.print("s, t_cruise=");
  Serial.print(t_cruise, 3);
  Serial.println("s");
  
  if (t_accel <= 0 || t_cruise < 0) {
    // Triangular profile - can't reach target velocity
    // Peak velocity for triangular: V_peak = 2*D/T
    float peakSpeed = (2.0 * totalSteps) / totalTime;
    float t_ramp = totalTime / 2.0;
    float accel = peakSpeed / t_ramp;
    
    executeTriangular(totalSteps, peakSpeed, accel, t_ramp);
  } else {
    // Trapezoidal profile
    float accel = maxSpeed / t_accel;
    int accelSteps = (int)(0.5 * accel * t_accel * t_accel);
    int cruiseSteps = (int)(maxSpeed * t_cruise);
    int decelSteps = totalSteps - accelSteps - cruiseSteps;
    
    executeTrapezoidal(accelSteps, cruiseSteps, decelSteps, maxSpeed, accel, t_accel, t_cruise);
  }
}

void executeTrapezoidal(int accelSteps, int cruiseSteps, int decelSteps, float maxSpeed, float accel, float t_accel_expected, float t_cruise_expected) {
  unsigned long stepDelayMicros;
  float currentSpeed;
  unsigned long startTime, accelTime, cruiseTime, decelTime;
  int totalSteps = accelSteps + cruiseSteps + decelSteps;
  
  // Acceleration phase
  startTime = micros();
  for (int i = 0; i < accelSteps; i++) {
    float t = sqrt((2.0 * i) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
  }
  accelTime = micros();
  
  // Cruise phase
  stepDelayMicros = (unsigned long)(1000000.0 / maxSpeed);
  for (int i = 0; i < cruiseSteps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
  }
  cruiseTime = micros();
  
  // Deceleration phase
  for (int i = 0; i < decelSteps; i++) {
    int stepsRemaining = decelSteps - i;
    float t = sqrt((2.0 * stepsRemaining) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
  }
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
  int halfSteps = totalSteps / 2;
  unsigned long stepDelayMicros;
  float currentSpeed;
  unsigned long startTime, accelTime, decelTime;
  
  // Acceleration phase
  startTime = micros();
  for (int i = 0; i < halfSteps; i++) {
    float t = sqrt((2.0 * i) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
  }
  accelTime = micros();
  
  // Deceleration phase
  int remainingSteps = totalSteps - halfSteps;
  for (int i = 0; i < remainingSteps; i++) {
    int stepsRemaining = remainingSteps - i;
    float t = sqrt((2.0 * stepsRemaining) / accel);
    currentSpeed = accel * t;
    if (currentSpeed < 1.0) currentSpeed = 1.0;
    stepDelayMicros = (unsigned long)(1000000.0 / currentSpeed);
    
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros / 2);
  }
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

void rotate(float revolutions, int rpm) {
  int totalSteps = revolutions * STEPS_PER_REV;
  int stepDelay = (60000000 / (rpm * STEPS_PER_REV)) / 2; // microseconds
  
  digitalWrite(DIR_PIN, revolutions > 0 ? HIGH : LOW);
  totalSteps = abs(totalSteps);
  
  for (int i = 0; i < totalSteps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
  }
}
