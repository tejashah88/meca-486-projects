// display.cpp
// Motor status rendering — reads motor state via public getters, calls LCD driver primitives.

#include "display.h"
#include "../../motor/motor_base.h"
#include "../../motor/linear_motor.h"
#include "../../driver/lcd/lcd.h"
#include "../../util/fstr.h"

namespace Display {

// ── Shared position-row helper ────────────────────────────────────────────────

static void renderPositionRow(MotorBase& m) {
  char line[17];
  if (m.mmPerRev() > 0.0f) {
    snprintf(line, sizeof(line), "M%d Pos:%smm", m.id(), fstr(m.positionRevs() * m.mmPerRev(), 1));
  } else {
    snprintf(line, sizeof(line), "M%d Pos:%srev", m.id(), fstr(m.positionRevs(), 2));
  }
  LCD::setCursor(0, 0);
  LCD::print(line);
}

// ── MotorBase overload ────────────────────────────────────────────────────────

void renderMotorInfo(MotorBase& m) {
  static unsigned long lastUpdate = 0;
  if (micros() - lastUpdate < 100000) return;
  lastUpdate = micros();

  renderPositionRow(m);
  LCD::setCursor(0, 1);
  LCD::print("  Motor Only    ");
}

// ── LinearMotor overload ──────────────────────────────────────────────────────

void renderMotorInfo(LinearMotor& m) {
  static unsigned long lastUpdate = 0;
  if (micros() - lastUpdate < 100000) return;
  lastUpdate = micros();

  renderPositionRow(m);

  LCD::setCursor(0, 1);
  bool end  = m.atEnd();
  bool home = m.atHome();
  if      (end && home) LCD::print("!!BOTH LIMITS!! ");
  else if (end)         LCD::print("** END  LIMIT **");
  else if (home)        LCD::print("** HOME LIMIT **");
  else                  LCD::print("   Status: OK   ");
}

} // namespace Display
