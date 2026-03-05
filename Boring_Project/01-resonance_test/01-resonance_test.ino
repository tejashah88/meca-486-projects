// Z-Axis Resonance Test — continuous run, speed controlled by two buttons
// Z-Axis (STR3): Dir=D24, Step=D25
// Btn UP: D22, Btn DOWN: D23
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include <LiquidCrystal.h>
#include "lib/driver/stepper/str3.h"
#include "lib/driver/lcd/lcd.h"
#include "lib/util/fstr.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);  // RS, EN, D4, D5, D6, D7
STR3 zDriver(24, 25, 3200);             // dirPin, stepPin, stepsPerRev

// ── CONFIGURATION ──────────────────────────────────────────────────────────────
const int   STEP_PIN    = 25;      // must match zDriver step pin
const int   BTN_UP      = 22;
const int   BTN_DOWN    = 23;

const float MIN_RPS     = 0.1f;
const float MAX_RPS     = 160.0f;
const float RPS_STEP    = 0.02f;
const int   DEBOUNCE_MS = 150;
// ──────────────────────────────────────────────────────────────────────────────

float         currentRPS     = 1.0f;
unsigned long lastStepMicros = 0;
bool          stepHigh       = false;
unsigned long lastUpMs       = 0;
unsigned long lastDownMs     = 0;

void setup() {
  Serial.begin(115200);

  zDriver.init();
  zDriver.setDirection(true);  // forward

  pinMode(BTN_UP,   INPUT);   // external pull-down: HIGH = pressed
  pinMode(BTN_DOWN, INPUT);

  LCD::init(&lcd, 16, 2);  // lcd, cols, rows
  LCD::clear();
  LCD::print("Z-Axis Run");
  LCD::setCursor(0, 1);  // col, row
  char buf[17];
  snprintf(buf, sizeof(buf), "RPS:%s", fstr(currentRPS, 2));  // fstr: val, decimals
  LCD::print(buf);

  Serial.println("=== Z-AXIS RESONANCE TEST ===");
  Serial.println("D22=UP  D23=DOWN");
  Serial.print("Start: "); Serial.print(currentRPS, 2); Serial.println(" RPS");
}

void loop() {
  unsigned long nowUs = micros();
  unsigned long nowMs = millis();
  int spr = zDriver.stepsPerRev();

  // ── Non-blocking step ──────────────────────────────────────────────────────
  // Driver's step() is blocking; toggle the pin directly here to stay responsive.
  unsigned long halfPeriod = (unsigned long)(500000.0f / (currentRPS * spr));
  if (nowUs - lastStepMicros >= halfPeriod) {
    stepHigh = !stepHigh;
    digitalWrite(STEP_PIN, stepHigh ? HIGH : LOW);
    lastStepMicros = nowUs;
  }

  // ── Button UP ──────────────────────────────────────────────────────────────
  if (digitalRead(BTN_UP) == HIGH && nowMs - lastUpMs > DEBOUNCE_MS) {
    lastUpMs = nowMs;
    currentRPS += RPS_STEP;
    if (currentRPS > MAX_RPS) currentRPS = MAX_RPS;
    Serial.print("RPS: "); Serial.println(currentRPS, 2);
  }

  // ── Button DOWN ────────────────────────────────────────────────────────────
  if (digitalRead(BTN_DOWN) == HIGH && nowMs - lastDownMs > DEBOUNCE_MS) {
    lastDownMs = nowMs;
    currentRPS -= RPS_STEP;
    if (currentRPS < MIN_RPS) currentRPS = MIN_RPS;
    Serial.print("RPS: "); Serial.println(currentRPS, 2);
  }

  // ── LCD update @ 10 Hz ─────────────────────────────────────────────────────
  static unsigned long lastLCDMs = 0;
  if (nowMs - lastLCDMs >= 100) {
    lastLCDMs = nowMs;
    char buf[17];
    snprintf(buf, sizeof(buf), "%s RPS  ", fstr(currentRPS, 2));  // fstr: val, decimals
    LCD::setCursor(0, 0);                                          // col, row
    LCD::print(buf);
    snprintf(buf, sizeof(buf), "%ld Hz      ", (long)(currentRPS * spr));
    LCD::setCursor(0, 1);  // col, row
    LCD::print(buf);
  }
}
