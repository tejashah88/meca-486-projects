// X-Axis Linear Stage — Max Acceleration Test
// THK KR33 + STR3. Dir=D51, Step=D53, Limits=D2(End)/D3(Home)
// Button=D22 (INPUT_PULLUP, active LOW) | LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11
//
// Fixed cruise speed and cruise distance; ramp distance decreases each level → accel increases.
// accel (rev/s²) = cruiseRPS² / (2 * rampRevs)
// Note the first level that stalls — the previous level is the max safe accel.

#include <LiquidCrystal.h>
#include "lib/driver/stepper/str3.h"
#include "lib/motor/linear_motor.h"
#include "lib/driver/lcd/lcd.h"
#include "lib/util/fstr.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
STR3        xDriver(51, 53, 200);
LinearMotor motor;

const int   BUTTON_PIN  = 22;
const float CRUISE_RPS  = 10.0f;
const float CRUISE_REVS = 5.0f;

float     ramps[]    = { 3.0f, 2.0f, 1.5f, 1.0f, 0.75f, 0.5f, 0.35f, 0.25f, 0.15f, 0.10f, 0.05f };
const int numLevels  = sizeof(ramps) / sizeof(ramps[0]);

void waitForButton() {
  while (digitalRead(BUTTON_PIN) == HIGH) delay(10);  // wait for press  (active LOW)
  while (digitalRead(BUTTON_PIN) == LOW)  delay(10);  // wait for release
  delay(50);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  motor.init(1, &xDriver, 2, 3, 6.0f, 15.0f);
  motor.enableLimits();

  LCD::init(&lcd, 16, 2);
  LCD::clear();
  LCD::print("Accel Test");
  LCD::setCursor(0, 1);
  LCD::print("Calibrating...");

  motor.calibrate(1.5f);
  motor.goHome(10.0f);

  Serial.println("=== X STAGE MAX ACCELERATION TEST ===");
  Serial.print("Cruise: "); Serial.print(CRUISE_RPS); Serial.print(" RPS, ");
  Serial.print(CRUISE_REVS); Serial.println(" rev");
  Serial.println("Press button to begin.\n");
  LCD::clear();
  LCD::print("Press to start");
}

void loop() {
  waitForButton();
  Serial.println("\n=== MAX ACCEL TEST START ===\n");

  for (int i = 0; i < numLevels; i++) {
    float ramp       = ramps[i];
    float accelRevS2 = (CRUISE_RPS * CRUISE_RPS) / (2.0f * ramp);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numLevels); Serial.print("]  ");
    Serial.print("Ramp="); Serial.print(ramp, 2); Serial.print(" rev  =>  ");
    Serial.print("Accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    LCD::clear();
    char buf[17];
    snprintf(buf, sizeof(buf), "Accel %d/%d", i + 1, numLevels);
    LCD::print(buf);
    LCD::setCursor(0, 1);
    snprintf(buf, sizeof(buf), "%s rev/s^2", fstr(accelRevS2, 0));
    LCD::print(buf);

    motor.manualTrapMove(ramp, CRUISE_REVS, ramp, CRUISE_RPS);
    delay(300);
    motor.goHome(10.0f);
    delay(300);

    if (i < numLevels - 1) {
      Serial.println("  >> Press for next level\n");
      LCD::clear();
      LCD::print("Press for next");
      waitForButton();
    }
  }

  Serial.println("\n=== MAX ACCEL TEST COMPLETE ===\n");
  LCD::clear();
  LCD::print("Done!");
  LCD::setCursor(0, 1);
  LCD::print("Press to repeat");
}
