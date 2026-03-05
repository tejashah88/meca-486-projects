// str3.cpp
// STR3 stepper driver — pin init and direction control.

#include "str3.h"

namespace STR3 {

void init(MotorConfig* m,
          uint8_t id,
          bool    hasLimits,
          int     dirPin,
          int     stepPin,
          int     stepsPerRev,
          bool    invertDir,
          int     limitEndPin,
          int     limitHomePin,
          float   mmPerRev,
          float   maxRPS) {
  m->id           = id;
  m->hasLimits    = hasLimits;
  m->invertDir    = invertDir;
  m->dirPin       = dirPin;
  m->stepPin      = stepPin;
  m->stepsPerRev  = stepsPerRev;
  m->limitEndPin  = limitEndPin;
  m->limitHomePin = limitHomePin;
  m->mmPerRev     = mmPerRev;
  m->maxRPS       = maxRPS;
  m->limitStopRevs     = 2.0f;  // max stop distance at maxRPS; lower speeds scale as v²/vmax²
  m->position          = 0;
  m->endPos            = 0;
  m->axisLength        = 0;
  m->speedRPS          = 0.0f;
  m->limitEndFlag      = false;
  m->limitHomeFlag     = false;

  pinMode(dirPin,  OUTPUT);
  pinMode(stepPin, OUTPUT);

  if (hasLimits) {
    // INPUT_PULLUP ensures NPN open-collector output reads reliably HIGH when not triggered
    if (limitEndPin  >= 0) pinMode(limitEndPin,  INPUT_PULLUP);
    if (limitHomePin >= 0) pinMode(limitHomePin, INPUT_PULLUP);
  }
}

void setDir(MotorConfig* m, bool forward) {
  digitalWrite(m->dirPin, (forward ^ m->invertDir) ? LOW : HIGH);
}

} // namespace STR3
