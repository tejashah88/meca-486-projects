// TrapezoidalMove.h
// Trapezoidal motion profile for stepper motors

#ifndef TRAPEZOIDAL_MOVE_H
#define TRAPEZOIDAL_MOVE_H

#include <Arduino.h>

// Pin definitions
extern const int DIR_PIN;
extern const int STEP_PIN;
extern const int STEPS_PER_REV;
extern const int SENSOR_PIN_1;
extern const int SENSOR_PIN_2;

// Trapezoidal move defined by total time and max RPS
void trapezoidalMove(float revolutions, float maxRPS, float totalTime);

// Profile move defined by phase distances (revolutions) and cruise RPS
// accelRevs: distance to ramp up to cruiseRPS
// cruiseRevs: distance at constant speed
// decelRevs: distance to slow down to stop
void profileMove(float accelRevs, float cruiseRevs, float decelRevs, float cruiseRPS);

// Helper functions (called internally)
void executeTrapezoidal(int accelSteps, int cruiseSteps, int decelSteps, 
                       float maxSpeed, float accel, 
                       float t_accel_expected, float t_cruise_expected);

void executeTriangular(int totalSteps, float peakSpeed, 
                      float accel, float t_ramp);

// Returns true if any limit switch is triggered (active LOW)
bool limitTriggered();

// Creep toward home (SENSOR_PIN_2) until triggered, then stop
// Assumes negative direction (DIR_PIN HIGH) moves toward home
void homeAxis(float slowRPS);

// Creep toward end (SENSOR_PIN_1) until triggered, then stop
// Assumes positive direction (DIR_PIN LOW) moves toward end
void findEnd(float slowRPS);

// Simple constant velocity rotation (not currently used)
void rotate(float revolutions, float rps);

#endif
