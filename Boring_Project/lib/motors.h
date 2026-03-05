// motors.h
// Single-include facade for the full motor control library.
// Include this header in your sketch instead of the individual module headers.
//
// Provides:
//   STR3::init(), STR3::setDir()
//   LimitSw::attach(), LimitSw::detach(), LimitSw::endActive(), LimitSw::homeActive()
//   LCD::init(), LCD::clear(), LCD::setCursor(), LCD::print()
//   Display::updateMotor()
//   MotionProf::profileMove(), MotionProf::trapezoidalMove(), MotionProf::rotate()
//   Cal::homeAxis(), Cal::findEnd(), Cal::calibrateAxis(), Cal::moveToHome(), Cal::moveToEnd()

#pragma once

#include "common/motor_types.h"
#include "driver/str3/str3.h"
#include "driver/limit_sw/limit_sw.h"
#include "driver/lcd/lcd.h"
#include "control/display/display.h"
#include "control/motion_prof/motion_prof.h"
#include "control/calibration/calibration.h"
