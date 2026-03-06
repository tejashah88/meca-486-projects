// X-Axis Linear Stage — Max Speed Test
// THK KR33 + STR3. Dir=D51, Step=D53, Limits=D2(End)/D3(Home)
// Button=D22 (INPUT_PULLUP, active LOW) | LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11
//
// Strategy: cruise distance is fixed at CRUISE_REVS revolutions.
//   rampRevs  = (travelRevs - CRUISE_REVS) / 2   ← maximises ramp = gentlest possible accel
//   accel (rev/s²) = rps² / (2 * rampRevs)        ← printed for info
//
// The limiting factor is top speed, not acceleration.

#include <LiquidCrystal.h>
#include "lib/driver/stepper/str3.h"
#include "lib/motor/linear_motor.h"
#include "lib/driver/lcd/lcd.h"
#include "lib/util/fstr.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);  // RS, EN, D4, D5, D6, D7
STR3        xDriver(51, 53, 200);       // dirPin, stepPin, stepsPerRev
LinearMotor motor;

const int   BUTTON_PIN        = 22;
const float CRUISE_REVS       = 1.0f;  // fixed cruise distance — all remaining travel goes to ramps
const float STAGE_MARGIN_REVS = 1.0f;  // rev kept clear at each end

float     speedLevels[]  = { 15.0f, 16.0f, 18.0f, 20.0f, 21.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f };
const int numSpeedLevels = sizeof(speedLevels) / sizeof(speedLevels[0]);

void waitForButton() {
  while (digitalRead(BUTTON_PIN) == HIGH) delay(10);  // wait for press  (active LOW)
  while (digitalRead(BUTTON_PIN) == LOW)  delay(10);  // wait for release
  delay(50);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  motor.init(1, &xDriver, 2, 3, 6.0f, 35.0f);  // id, driver, limitEndPin, limitHomePin, mmPerRev, maxRPS
  motor.enableLimits();

  LCD::init(&lcd, 16, 2);  // lcd, cols, rows
  LCD::clear();
  LCD::print("Speed Test");
  LCD::setCursor(0, 1);  // col, row
  LCD::print("Calibrating...");

  motor.calibrate(1.5f);   // slowRPS
  motor.goHome(15.0f);     // cruiseRPS

  float travelRevs = motor.axisLengthRevs() - 2.0f * STAGE_MARGIN_REVS;
  Serial.println("=== X STAGE MAX SPEED TEST ===");
  Serial.print("Stage travel available: "); Serial.print(travelRevs, 2); Serial.println(" rev");
  Serial.print("Cruise fixed at: "); Serial.print(CRUISE_REVS); Serial.println(" rev");
  Serial.println("Press button to begin.\n");
  LCD::clear();
  LCD::print("Press to start");
}

void loop() {
  waitForButton();

  float travelRevs = motor.axisLengthRevs() - 2.0f * STAGE_MARGIN_REVS;

  if (travelRevs < 3.0f) {
    Serial.println("ERROR: axis travel too short — recalibrate.");
    LCD::clear();
    LCD::print("Recalibrate!");
    return;
  }

  Serial.println("\n=== MAX SPEED TEST START ===\n");

  for (int i = 0; i < numSpeedLevels; i++) {
    float rps      = speedLevels[i];
    float rampRevs = (travelRevs - CRUISE_REVS) / 2.0f;

    if (rampRevs < 0.1f) {
      Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numSpeedLevels);
      Serial.print("]  "); Serial.print(rps, 1);
      Serial.println(" RPS — SKIP (stage too short for cruise + ramps)");
      continue;
    }

    float accelRevS2 = (rps * rps) / (2.0f * rampRevs);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numSpeedLevels); Serial.print("]  ");
    Serial.print(rps, 1); Serial.print(" RPS  |  ramp="); Serial.print(rampRevs, 2);
    Serial.print(" rev  cruise="); Serial.print(CRUISE_REVS, 1);
    Serial.print(" rev  accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    LCD::clear();
    char buf[17];
    snprintf(buf, sizeof(buf), "Spd %d/%d %sRPS", i + 1, numSpeedLevels, fstr(rps, 1));  // fstr: val, decimals
    LCD::print(buf);
    LCD::setCursor(0, 1);                                                                   // col, row
    snprintf(buf, sizeof(buf), "a=%s r/s2", fstr(accelRevS2, 0));                          // fstr: val, decimals
    LCD::print(buf);

    motor.manualTrapMove(rampRevs, CRUISE_REVS, rampRevs, rps);  // accelRevs, cruiseRevs, decelRevs, cruiseRPS
    delay(300);
    motor.goHome(15.0f);  // cruiseRPS
    delay(300);

    if (i < numSpeedLevels - 1) {
      Serial.println("  >> Press for next level\n");
      LCD::clear();
      LCD::print("Press for next");
      waitForButton();
    }
  }

  Serial.println("\n=== MAX SPEED TEST COMPLETE ===\n");
  LCD::clear();
  LCD::print("Done!");
  LCD::setCursor(0, 1);  // col, row
  LCD::print("Press to repeat");
}
