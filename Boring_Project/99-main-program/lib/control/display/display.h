// display.h
// Motor status rendering for the LCD.
// Uses overloads so LinearMotor shows limit switch status; MotorBase shows position only.
// The LCD must be initialised via LCD::init() before calling these functions.

#pragma once

class MotorBase;
class LinearMotor;

namespace Display {

  // Write base motor state to the LCD (rate-limited to 10 Hz).
  // Row 0: position in mm (if mmPerRev > 0) or revolutions.
  // Row 1: "Motor Only" — no limit switch information.
  void renderMotorInfo(MotorBase& m);

  // Write linear axis state to the LCD (rate-limited to 10 Hz).
  // Row 0: position in mm (if mmPerRev > 0) or revolutions.
  // Row 1: limit switch status — OK / HOME LIMIT / END LIMIT / BOTH.
  void renderMotorInfo(LinearMotor& m);

} // namespace Display
