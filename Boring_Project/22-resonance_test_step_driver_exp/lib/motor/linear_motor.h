// linear_motor.h
// Linear axis stepper with limit switches and calibration.
// Extends MotorBase with homing, end-finding, and position-based traversal.

#pragma once

#include "motor_base.h"

class LinearMotor : public MotorBase {
public:
  // Extended init — driver plus limit switch pins and axis specs.
  // mmPerRev: lead-screw pitch (mm/rev); pass 0 to suppress mm output.
  // maxRPS: operating ceiling used to compute the limit-triggered decel rate.
  void init(uint8_t id, StepperDriver* driver,
            int limitEndPin, int limitHomePin, float mmPerRev, float maxRPS);

  // Register FALLING-edge interrupts on limitEndPin and limitHomePin.
  // Finds a free ISR slot (max 4 LinearMotors). Call before any trapezoidal moves.
  void enableLimits();

  // Detach interrupts and release the ISR slot.
  void disableLimits();

  // Live pin read — true when the sensor is currently active (pin LOW).
  bool atEnd()  const;
  bool atHome() const;

  // Creep toward the home sensor at slowRPS, back off until clear. Sets position = 0.
  void findHome(float slowRPS);

  // Creep toward the end sensor at slowRPS. Records endPos and axisLength.
  void findEnd(float slowRPS);

  // Full calibration sequence: findHome then findEnd.
  // Prints axis length in steps, revolutions, and mm over Serial.
  void calibrate(float slowRPS);

  // Trapezoidal move from current position back to step 0 (home).
  void goHome(float cruiseRPS);

  // Trapezoidal move from current position to endPos.
  void goToEnd(float cruiseRPS);

  // Axis length after calibration.
  float axisLengthRevs() const { return (float)_axisLength / _stepsPerRev; }
  float axisLengthMM()   const { return axisLengthRevs() * _mmPerRev; }

protected:
  long _endPos, _axisLength;

private:
  void creepUntilSensor(int sensorPin, int8_t dir, float rps);
  void creepUntilSensorClear(int sensorPin, int8_t dir, float rps);
};
