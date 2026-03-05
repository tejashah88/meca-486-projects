// X-Axis Linear Stage — Max Acceleration Test
// THK KR33 + STR3. Dir=D51, Step=D53, Button=D22, Limits D2(End)/D3(Home), LCD: 7,8,4,5,6,11
//
// Fixed cruise speed and cruise distance; ramp distance decreases each level → accel increases.
// accel (rev/s²) = cruiseRPS² / (2 * rampRevs)
// Note the first level that stalls — the previous level is the max safe accel.

#include "TrapezoidalMove.h"

const int BUTTON_PIN = 22;

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

const float CRUISE_RPS   = 10.0f;
const float CRUISE_REVS  = 5.0f;

float ramps[]    = { 3.0f, 2.0f, 1.5f, 1.0f, 0.75f, 0.5f, 0.35f, 0.25f, 0.15f, 0.10f, 0.05f };
const int numLevels = sizeof(ramps) / sizeof(ramps[0]);

void waitForButton() {
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); }
  delay(50);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Accel Test");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  motorInit(&motor, 1, true, 51, 53, 200, false, &lcd, 2, 3, 6.0f, -1, 100, 15.0f);
  attachLimitInterrupts(&motor);

  calibrateAxis(&motor, 1.5f);
  moveToHome(&motor, 10.0f);

  Serial.println("=== X STAGE MAX ACCELERATION TEST ===");
  Serial.print("Cruise: "); Serial.print(CRUISE_RPS); Serial.print(" RPS, ");
  Serial.print(CRUISE_REVS); Serial.println(" rev");
  Serial.println("Press button to begin.\n");
  lcd.clear();
  lcd.print("Press to start");
}

void loop() {
  updateLCD(&motor);
  waitForButton();

  Serial.println("\n=== MAX ACCEL TEST START ===\n");

  for (int i = 0; i < numLevels; i++) {
    float ramp       = ramps[i];
    int   aSteps     = (int)(ramp * motor.stepsPerRev);
    float vSteps     = CRUISE_RPS * motor.stepsPerRev;
    float accelRevS2 = (vSteps * vSteps) / (2.0f * (float)aSteps * motor.stepsPerRev);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numLevels); Serial.print("]  ");
    Serial.print("Ramp="); Serial.print(ramp, 2); Serial.print(" rev  =>  ");
    Serial.print("Accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    lcd.clear();
    lcd.print("Accel "); lcd.print(i + 1); lcd.print("/"); lcd.print(numLevels);
    lcd.setCursor(0, 1);
    lcd.print(accelRevS2, 0); lcd.print(" rev/s^2");

    profileMove(&motor, ramp, CRUISE_REVS, ramp, CRUISE_RPS);
    delay(300);
    moveToHome(&motor, 10.0f);
    delay(300);

    if (i < numLevels - 1) {
      Serial.println("  >> Press for next level\n");
      lcd.clear(); lcd.print("Press for next");
      waitForButton();
    }
  }

  Serial.println("\n=== MAX ACCEL TEST COMPLETE ===\n");
  lcd.clear(); lcd.print("Done!");
  lcd.setCursor(0, 1); lcd.print("Press to repeat");
}
