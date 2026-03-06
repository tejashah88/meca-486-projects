// Motor-Only Stall & Resonance Test — STR3 + Rotating Mass
// No limit switches; uses RotationalMotor (no homing or calibration).
// SHORT press (< 1s) → Resonance test
// LONG press  (≥ 1s) → Stall test
// Direction: D9, Step: D10, Button: D22 (INPUT_PULLUP, active LOW)
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include <LiquidCrystal.h>
#include "lib/driver/stepper/str3.h"
#include "lib/motor/rotational_motor.h"
#include "lib/driver/lcd/lcd.h"
#include "lib/control/display/display.h"
#include "lib/util/fstr.h"

const int BUTTON_PIN = 22;

// ── CHANGE TO MATCH YOUR DRIVER ──────────────────────────────────────────────
const int SPR = 200;  // steps/rev: 200,400,800,1000,1600,2000,3200...

const float MAX_RPS     = 1000000.0f / (2.0f * 5.0f) / SPR;  // limited by min pulse width (5 µs)
const float CRUISE_RPS  = 10.0f;
const float CRUISE_REVS = 5.0f;
const float ROT_REVS    = 3.0f;
// ─────────────────────────────────────────────────────────────────────────────

float stallRamps[]   = { 3.0, 2.0, 1.5, 1.0, 0.75, 0.5, 0.35, 0.25, 0.15, 0.10, 0.05 };
int   numStallLevels = sizeof(stallRamps) / sizeof(stallRamps[0]);

const float ALL_SPEEDS[] = {
  0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90,
  1.00, 1.25, 1.50, 1.75, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00,
  6.00, 7.00, 8.00, 9.00, 10.0, 12.0, 15.0, 18.0, 20.0,
  25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 75.0, 100.0
};
const int TOTAL_SPEEDS = sizeof(ALL_SPEEDS) / sizeof(ALL_SPEEDS[0]);

LiquidCrystal   lcd(7, 8, 4, 5, 6, 11);  // RS, EN, D4, D5, D6, D7
STR3            driver(9, 10, SPR);        // dirPin, stepPin, stepsPerRev
RotationalMotor motor;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Starts timing when called (button already pressed), returns when released.
unsigned long measurePressDuration() {
  unsigned long t = millis();
  while (digitalRead(BUTTON_PIN) == LOW) { delay(5); }  // wait for release (active LOW)
  delay(50);
  return millis() - t;
}

void waitForButtonPress() {
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); Display::renderMotorInfo(motor); }  // drain current press
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); Display::renderMotorInfo(motor); }  // wait for press
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); }                                    // wait for release
  delay(50);
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  motor.init(1, &driver);    // id, driver
  LCD::init(&lcd, 16, 2);    // lcd, cols, rows
  LCD::clear();
  LCD::print("Motor-Only Test");
  LCD::setCursor(0, 1);      // col, row
  char buf[17];
  snprintf(buf, sizeof(buf), "SPR:%d", SPR);
  LCD::print(buf);

  Serial.println("=== MOTOR-ONLY TEST ===");
  Serial.print("SPR="); Serial.print(SPR);
  Serial.print("  MaxRPS="); Serial.println(MAX_RPS, 1);
  Serial.println("SHORT press = Resonance | LONG press = Stall\n");

  delay(1000);
  LCD::setCursor(0, 1);      // col, row
  LCD::print("S=Res  L=Stall");
}

// ── Resonance test ────────────────────────────────────────────────────────────

void resonanceTest() {
  Serial.println("=== RESONANCE TEST START ===\n");
  char buf[17];
  int  lvl = 0;

  for (int i = 0; i < TOTAL_SPEEDS; i++) {
    float rps    = ALL_SPEEDS[i];
    float stepHz = rps * SPR;

    if (rps > MAX_RPS) {
      Serial.print("[ SKIP ] "); Serial.print(rps, 2); Serial.println(" RPS");
      continue;
    }

    lvl++;
    Serial.print("[Lv"); Serial.print(lvl); Serial.print("]  ");
    Serial.print(rps, 2); Serial.print(" RPS  ");
    Serial.print(stepHz, 0); Serial.println(" Hz    Vibration: ___");

    LCD::clear();
    LCD::setCursor(0, 0);  // col, row
    snprintf(buf, sizeof(buf), "%s RPS", fstr(rps, 2));          // fstr: val, decimals
    LCD::print(buf);
    LCD::setCursor(0, 1);  // col, row
    snprintf(buf, sizeof(buf), "%dHz Lv%d", (int)stepHz, lvl);
    LCD::print(buf);

    motor.spinRevs( ROT_REVS, rps);   // revolutions, rps
    delay(150);
    motor.spinRevs(-ROT_REVS, rps);   // revolutions, rps
    delay(150);

    if (i < TOTAL_SPEEDS - 1 && ALL_SPEEDS[i + 1] <= MAX_RPS) {
      LCD::clear();
      LCD::print("Note vibration");
      LCD::setCursor(0, 1);  // col, row
      LCD::print("Press for next");
      waitForButtonPress();
    }
  }
  Serial.println("=== RESONANCE COMPLETE ===\n");
}

// ── Stall test ────────────────────────────────────────────────────────────────

void stallTest() {
  Serial.println("=== STALL TEST START ===\n");
  char buf[17];

  for (int i = 0; i < numStallLevels; i++) {
    float ramp       = stallRamps[i];
    float accelRevS2 = (CRUISE_RPS * CRUISE_RPS) / (2.0f * ramp);  // rev/s²

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numStallLevels); Serial.print("]  ");
    Serial.print("Ramp="); Serial.print(ramp, 2); Serial.print(" rev    ");
    Serial.print("Accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    LCD::clear();
    LCD::setCursor(0, 0);  // col, row
    snprintf(buf, sizeof(buf), "Stall Lv%d", i + 1);
    LCD::print(buf);
    LCD::setCursor(0, 1);  // col, row
    snprintf(buf, sizeof(buf), "%s rev/s^2", fstr(accelRevS2, 0));  // fstr: val, decimals
    LCD::print(buf);

    motor.manualTrapMove( ramp,  CRUISE_REVS,  ramp, CRUISE_RPS);  // accelRevs, cruiseRevs, decelRevs, cruiseRPS
    delay(300);
    motor.manualTrapMove(-ramp, -CRUISE_REVS, -ramp, CRUISE_RPS);  // accelRevs, cruiseRevs, decelRevs, cruiseRPS
    delay(300);

    if (i < numStallLevels - 1) {
      Serial.println("  >> Press for next level\n");
      LCD::clear();
      LCD::print("Pass/Fail?");
      LCD::setCursor(0, 1);  // col, row
      LCD::print("Press for next");
      waitForButtonPress();
    }
  }
  Serial.println("=== STALL COMPLETE ===\n");
}

// ── Main loop ─────────────────────────────────────────────────────────────────

void loop() {
  Display::renderMotorInfo(motor);

  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); Display::renderMotorInfo(motor); }  // drain press
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); Display::renderMotorInfo(motor); }  // wait for press

  if (measurePressDuration() >= 1000) {
    LCD::clear(); LCD::print("STALL TEST");
    LCD::setCursor(0, 1);  // col, row
    LCD::print("Starting...");
    delay(400);
    stallTest();
  } else {
    LCD::clear(); LCD::print("RESONANCE TEST");
    LCD::setCursor(0, 1);  // col, row
    LCD::print("Starting...");
    delay(400);
    resonanceTest();
  }

  LCD::clear(); LCD::print("Done!");
  LCD::setCursor(0, 1);  // col, row
  LCD::print("S=Res  L=Stall");
}
