// display.cpp
// Motor status rendering — reads MotorConfig, calls LCD driver primitives.

#include "display.h"
#include "../../driver/lcd/lcd.h"

namespace Display {

void updateMotor(MotorConfig* m) {
  static unsigned long lastUpdate = 0;
  if (micros() - lastUpdate < 100000) return;  // max 10 Hz
  lastUpdate = micros();

  // Row 0: position in mm (if pitch known) or revolutions
  char posStr[8];
  char unit[4];
  if (m->mmPerRev > 0.0f) {
    dtostrf((float)m->position / m->stepsPerRev * m->mmPerRev, 6, 1, posStr);
    strncpy(unit, "mm", sizeof(unit));
  } else {
    dtostrf((float)m->position / m->stepsPerRev, 6, 2, posStr);
    strncpy(unit, "rev", sizeof(unit));
  }
  char line[17];
  snprintf(line, sizeof(line), "M%d Pos:%s%s", m->id, posStr, unit);
  LCD::setCursor(0, 0);
  LCD::print(line);

  // Row 1: limit switch status (digitalRead for live state; flags clear between moves)
  LCD::setCursor(0, 1);
  if (m->hasLimits) {
    bool atEnd  = (digitalRead(m->limitEndPin)  == LOW);
    bool atHome = (digitalRead(m->limitHomePin) == LOW);
    if      (atEnd && atHome) LCD::print("!!BOTH LIMITS!! ");
    else if (atEnd)           LCD::print("** END  LIMIT **");
    else if (atHome)          LCD::print("** HOME LIMIT **");
    else                      LCD::print("   Status: OK   ");
  } else {
    LCD::print("  Motor Only    ");
  }
}

} // namespace Display
