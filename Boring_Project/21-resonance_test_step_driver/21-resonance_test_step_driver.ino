// STR3 Stepper Resonance Test — continuous run, speed controlled by two buttons
// STR3: Dir=D51, Step=D53, 200 spr
// Btn UP: D22, Btn DOWN: D23

#include "lib/driver/stepper/str3.h"

STR3 driver(51, 53, 200);  // dirPin, stepPin, stepsPerRev

// ── CONFIGURATION ──────────────────────────────────────────────────────────────
const int   BTN_UP      = 22;
const int   BTN_DOWN    = 23;

const float MIN_HZ      = 1.0f;
const float MAX_HZ      = 200.0f;
const float HZ_STEP     = 1.0f;
const int   DEBOUNCE_MS = 150;
// ──────────────────────────────────────────────────────────────────────────────

float         currentHz      = 150.0f;
unsigned long lastStepMicros = 0;
unsigned long lastUpMs       = 0;
unsigned long lastDownMs     = 0;

void setup() {
  Serial.begin(115200);

  driver.init();
  driver.setDirection(true);

  pinMode(BTN_UP,   INPUT);   // external pull-down: HIGH = pressed
  pinMode(BTN_DOWN, INPUT);

  Serial.println("=== STR3 STEPPER RESONANCE TEST ===");
  Serial.println("D22=UP  D23=DOWN");
  Serial.print("Start: "); Serial.print(currentHz, 1); Serial.println(" Hz");
}

void loop() {
  unsigned long nowUs = micros();
  unsigned long nowMs = millis();

  // ── Non-blocking step ──────────────────────────────────────────────────────
  // Timing is managed here; driver.step(2) sends a minimal 1 µs pulse.
  unsigned long stepPeriod = (unsigned long)(1000000.0f / currentHz);
  if (nowUs - lastStepMicros >= stepPeriod) {
    driver.step(2);
    lastStepMicros = nowUs;
  }

  // ── Button UP ──────────────────────────────────────────────────────────────
  if (digitalRead(BTN_UP) == HIGH && nowMs - lastUpMs > DEBOUNCE_MS) {
    lastUpMs = nowMs;
    currentHz += HZ_STEP;
    if (currentHz > MAX_HZ) currentHz = MAX_HZ;
    Serial.print("Hz: "); Serial.println(currentHz, 1);
  }

  // ── Button DOWN ────────────────────────────────────────────────────────────
  if (digitalRead(BTN_DOWN) == HIGH && nowMs - lastDownMs > DEBOUNCE_MS) {
    lastDownMs = nowMs;
    currentHz -= HZ_STEP;
    if (currentHz < MIN_HZ) currentHz = MIN_HZ;
    Serial.print("Hz: "); Serial.println(currentHz, 1);
  }
}
