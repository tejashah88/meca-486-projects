// TrapezoidalMove.h
// Trapezoidal motion profile for stepper motors

#ifndef TRAPEZOIDAL_MOVE_H
#define TRAPEZOIDAL_MOVE_H

#include <Arduino.h>
#include <LiquidCrystal.h>

// Pin definitions
extern const int DIR_PIN;
extern const int STEP_PIN;
extern const int STEPS_PER_REV;
extern const int SENSOR_PIN_1;
extern const int SENSOR_PIN_2;

// Position tracking (in steps; 0 = home)
extern long motorPosition;
extern long endPosition;
extern long axisLength;
extern float currentSpeedRPS;

// LCD object (defined in .ino)
extern LiquidCrystal lcd;

// Update LCD with current position and velocity (rate-limited to ~10 Hz)
void updateLCD();

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

// Creep toward home (SENSOR_PIN_2) until triggered; sets motorPosition = 0
void homeAxis(float slowRPS);

// Creep toward end (SENSOR_PIN_1) until triggered; sets endPosition and axisLength
void findEnd(float slowRPS);

// Home then find end; reports axis length
void calibrateAxis(float slowRPS);

// Trapezoidal move to home (position 0) or end position
void moveToHome(float cruiseRPS);
void moveToEnd(float cruiseRPS);

// Simple constant velocity rotation (not currently used)
void rotate(float revolutions, float rps);

#endif
