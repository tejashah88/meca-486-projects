// Stall Test — STR3 Stepper on THK KR33
// X-Axis (STR3): Dir=D9, Step=D10, Limits=D2(End)/D3(Home)
// Button: D22 | LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include <LiquidCrystal.h>
#include "lib/driver/stepper/str3.h"
#include "lib/motor/linear_motor.h"
#include "lib/driver/lcd/lcd.h"
#include "lib/util/fstr.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
STR3        xDriver(9, 10, 200);
LinearMotor motor;

const int   BUTTON_PIN  = 22;
const float CRUISE_RPS  = 10.0f;
const float CRUISE_REVS = 5.0f;

float ramps[]  = { 3.0, 2.0, 1.5, 1.0, 0.75, 0.5, 0.35, 0.25, 0.15, 0.10, 0.05 };
int   numTests = sizeof(ramps) / sizeof(ramps[0]);

void waitForButtonPress() {
  while (digitalRead(BUTTON_PIN) == LOW)  delay(10);
  while (digitalRead(BUTTON_PIN) == HIGH) delay(10);
  while (digitalRead(BUTTON_PIN) == LOW)  delay(10);
  delay(50);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  motor.init(1, &xDriver, 2, 3, 6.0f, 15.0f);
  motor.enableLimits();

  LCD::init(&lcd, 16, 2);
  LCD::clear();
  LCD::print("Stall Test");
  LCD::setCursor(0, 1);
  LCD::print("Calibrating...");

  motor.calibrate(0.5f);
  motor.goHome(2.0f);

  Serial.println("=== STALL TEST READY ===");
  Serial.print("Cruise: "); Serial.print(CRUISE_RPS); Serial.println(" RPS");
  LCD::clear();
  LCD::print("Press to begin");
}

void loop() {
  waitForButtonPress();
  Serial.println("=== STALL TEST START ===\n");

  for (int i = 0; i < numTests; i++) {
    float ramp       = ramps[i];
    float accelRevS2 = (CRUISE_RPS * CRUISE_RPS) / (2.0f * ramp);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numTests); Serial.print("]  ");
    Serial.print("Ramp="); Serial.print(ramp, 2); Serial.print(" rev    ");
    Serial.print("Accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    LCD::clear();
    char buf[17];
    snprintf(buf, sizeof(buf), "Level %d/%d", i + 1, numTests);
    LCD::print(buf);
    LCD::setCursor(0, 1);
    snprintf(buf, sizeof(buf), "%s rev/s^2", fstr(accelRevS2, 0));
    LCD::print(buf);

    motor.manualTrapMove(ramp, CRUISE_REVS, ramp, CRUISE_RPS);
    delay(300);
    motor.goHome(2.0f);
    delay(300);

    if (i < numTests - 1) {
      Serial.println("  >> Press for next level\n");
      LCD::clear();
      LCD::print("Press for next");
      waitForButtonPress();
    }
  }

  Serial.println("=== STALL TEST COMPLETE ===");
  LCD::clear();
  LCD::print("Done!");
  LCD::setCursor(0, 1);
  LCD::print("Press to repeat");
}
