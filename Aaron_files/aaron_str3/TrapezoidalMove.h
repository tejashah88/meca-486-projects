// TrapezoidalMove.h
// Trapezoidal motion profile for stepper motors

#ifndef TRAPEZOIDAL_MOVE_H
#define TRAPEZOIDAL_MOVE_H

#include <Arduino.h>

// Pin definitions
extern const int DIR_PIN;
extern const int STEP_PIN;
extern const int STEPS_PER_REV;

// Main function to execute a trapezoidal move
void trapezoidalMove(float revolutions, float maxRPM, float totalTime);

// Helper functions (called internally)
void executeTrapezoidal(int accelSteps, int cruiseSteps, int decelSteps, 
                       float maxSpeed, float accel, 
                       float t_accel_expected, float t_cruise_expected);

void executeTriangular(int totalSteps, float peakSpeed, 
                      float accel, float t_ramp);

// Simple constant velocity rotation (not currently used)
void rotate(float revolutions, int rpm);

#endif
