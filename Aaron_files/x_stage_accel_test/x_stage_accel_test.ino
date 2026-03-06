// X-Axis Linear Stage — Max Acceleration Test
// THK KR33 + STR3. Dir=D51, Step=D53, Button=D22, Limits D2(End)/D3(Home), LCD: 7,8,4,5,6,11
//
// Levels defined by acceleration (rev/s²). Ramp = v²/(2a).
// Step rounding is reported so you see the actual accel that ran.
// Note the first level that stalls — the previous level is the max safe accel.

#include "TrapezoidalMove.h"

const int BUTTON_PIN = 22;

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

const float CRUISE_RPS  = 15.0f;
const float CRUISE_REVS = 5.0f;

// Acceleration levels in rev/s² — coarse sweep first, fine near stall threshold
float accelLevels[] = {
   100,  150,  200,  250,  300,  350,  400,
   450,  500,  550,  600,  650,  700,  750,
   800,  900, 1000, 1200, 1500
};
const int numLevels = sizeof(accelLevels) / sizeof(accelLevels[0]);

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

  motorInit(&motor, 1, true, 51, 53, 200, false, &lcd, 2, 3, 6.0f, -1, 100, 20.0f);
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
    float targetAccel = accelLevels[i];

    // Compute ramp from target accel, then back-calculate actual accel after step rounding
    float rampRevs    = (CRUISE_RPS * CRUISE_RPS) / (2.0f * targetAccel);
    int   aSteps      = max(1, (int)(rampRevs * motor.stepsPerRev));
    float actualAccel = (CRUISE_RPS * CRUISE_RPS) / (2.0f * ((float)aSteps / motor.stepsPerRev));
    float actualRamp  = (float)aSteps / motor.stepsPerRev;

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numLevels); Serial.print("]  ");
    Serial.print("Target="); Serial.print(targetAccel, 0); Serial.print(" rev/s²  =>  ");
    Serial.print("Actual="); Serial.print(actualAccel, 1); Serial.print(" rev/s²  (");
    Serial.print(aSteps); Serial.print(" steps / "); Serial.print(actualRamp, 3); Serial.println(" rev ramp)");

    lcd.clear();
    lcd.print("Accel "); lcd.print(i + 1); lcd.print("/"); lcd.print(numLevels);
    lcd.setCursor(0, 1);
    lcd.print(actualAccel, 0); lcd.print(" r/s2");

    profileMove(&motor, actualRamp, CRUISE_REVS, actualRamp, CRUISE_RPS);
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
