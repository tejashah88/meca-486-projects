// limit_sw.cpp
// Limit switch ISR slot table — up to 4 motors.
// Each slot owns one end-ISR stub and one home-ISR stub.
// The stubs set the volatile flag on the registered MotorConfig.

#include "limit_sw.h"

namespace {

  static const int MAX_SLOTS = 4;
  static MotorConfig* _endMotors[MAX_SLOTS]  = {nullptr, nullptr, nullptr, nullptr};
  static MotorConfig* _homeMotors[MAX_SLOTS] = {nullptr, nullptr, nullptr, nullptr};

  static void endISR0()  { _endMotors[0]->limitEndFlag   = true; }
  static void homeISR0() { _homeMotors[0]->limitHomeFlag  = true; }
  static void endISR1()  { _endMotors[1]->limitEndFlag   = true; }
  static void homeISR1() { _homeMotors[1]->limitHomeFlag  = true; }
  static void endISR2()  { _endMotors[2]->limitEndFlag   = true; }
  static void homeISR2() { _homeMotors[2]->limitHomeFlag  = true; }
  static void endISR3()  { _endMotors[3]->limitEndFlag   = true; }
  static void homeISR3() { _homeMotors[3]->limitHomeFlag  = true; }

  typedef void (*IsrFunc)();
  static const IsrFunc endISRs[]  = {endISR0,  endISR1,  endISR2,  endISR3};
  static const IsrFunc homeISRs[] = {homeISR0, homeISR1, homeISR2, homeISR3};

} // anonymous namespace

namespace LimitSw {

void attach(MotorConfig* m) {
  if (!m->hasLimits) {
    Serial.println("LimitSw::attach: no limits configured.");
    return;
  }

  int slot = -1;
  for (int i = 0; i < MAX_SLOTS; i++) {
    if (_endMotors[i] == nullptr) { slot = i; break; }
  }
  if (slot < 0) {
    Serial.println("LimitSw::attach: no free ISR slots.");
    return;
  }

  _endMotors[slot]  = m;
  _homeMotors[slot] = m;

  if (m->limitEndPin  >= 0) attachInterrupt(digitalPinToInterrupt(m->limitEndPin),  endISRs[slot],  FALLING);
  if (m->limitHomePin >= 0) attachInterrupt(digitalPinToInterrupt(m->limitHomePin), homeISRs[slot], FALLING);

  m->limitEndFlag  = false;
  m->limitHomeFlag = false;

  Serial.print("LimitSw: ISRs attached for motor "); Serial.print(m->id);
  Serial.print(" (slot "); Serial.print(slot); Serial.println(")");
}

void detach(MotorConfig* m) {
  for (int i = 0; i < MAX_SLOTS; i++) {
    if (_endMotors[i] == m) {
      if (m->limitEndPin  >= 0) detachInterrupt(digitalPinToInterrupt(m->limitEndPin));
      if (m->limitHomePin >= 0) detachInterrupt(digitalPinToInterrupt(m->limitHomePin));
      _endMotors[i]  = nullptr;
      _homeMotors[i] = nullptr;
      return;
    }
  }
}

bool endActive(MotorConfig* m) {
  return m->limitEndPin >= 0 && digitalRead(m->limitEndPin) == LOW;
}

bool homeActive(MotorConfig* m) {
  return m->limitHomePin >= 0 && digitalRead(m->limitHomePin) == LOW;
}

} // namespace LimitSw
