// X-Axis Linear Stage — Max Speed Test
// THK KR33 + STR3. Dir=D51, Step=D53, Button=D22, Limits D2(End)/D3(Home), LCD: 7,8,4,5,6,11
//
// Strategy: cruise distance is fixed at CRUISE_REVS revolutions.
//   ramp_revs   = (stageTravel - CRUISE_REVS) / 2   ← maximises ramp = gentlest possible accel
//   accel (rev/s²) = rps² / (2 * ramp_revs)         ← printed for info
//
// The limiting factor is top speed, not acceleration.

#include "TrapezoidalMove.h"

const int   BUTTON_PIN        = 22;
const float CRUISE_REVS       = 1.0f; // fixed cruise distance — all remaining travel goes to ramps
const float STAGE_MARGIN_REVS = 1.0f; // rev kept clear at each end

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

float speedLevels[] = { 15.0f, 16.0f, 18.0f, 20.0f, 21.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f };
const int numSpeedLevels = sizeof(speedLevels) / sizeof(speedLevels[0]);

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
  lcd.print("Speed Test");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  motorInit(&motor, 1, true, 51, 53, 200, false, &lcd, 2, 3, 6.0f, -1, 100, 35.0f);
  attachLimitInterrupts(&motor);

  calibrateAxis(&motor, 1.5f);
  moveToHome(&motor, 15.0f);

  float travelRevs = ((float)motor.axisLength / motor.stepsPerRev) - 2.0f * STAGE_MARGIN_REVS;
  Serial.println("=== X STAGE MAX SPEED TEST ===");
  Serial.print("Stage travel available: "); Serial.print(travelRevs, 2); Serial.println(" rev");
  Serial.print("Cruise fixed at: "); Serial.print(CRUISE_REVS); Serial.println(" rev");
  Serial.println("Press button to begin.\n");
  lcd.clear();
  lcd.print("Press to start");
}

void loop() {
  updateLCD(&motor);
  waitForButton();

  float axisRevs   = (float)motor.axisLength / (float)motor.stepsPerRev;
  float travelRevs = axisRevs - 2.0f * STAGE_MARGIN_REVS;

  if (travelRevs < 3.0f) {
    Serial.println("ERROR: axis travel too short — recalibrate.");
    lcd.clear(); lcd.print("Recalibrate!");
    return;
  }

  Serial.println("\n=== MAX SPEED TEST START ===\n");

  for (int i = 0; i < numSpeedLevels; i++) {
    float rps        = speedLevels[i];
    float rampRevs   = (travelRevs - CRUISE_REVS) / 2.0f;

    if (rampRevs < 0.1f) {
      Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numSpeedLevels);
      Serial.print("]  "); Serial.print(rps, 1);
      Serial.println(" RPS — SKIP (stage too short for cruise + ramps)");
      continue;
    }

    float accelRevS2 = (rps * rps) / (2.0f * rampRevs);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numSpeedLevels);     Serial.print("]  ");
    Serial.print(rps, 1); Serial.print(" RPS  |  ramp="); Serial.print(rampRevs, 2);
    Serial.print(" rev  cruise="); Serial.print(CRUISE_REVS, 1);
    Serial.print(" rev  accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    lcd.clear();
    lcd.print("Spd "); lcd.print(i + 1); lcd.print("/"); lcd.print(numSpeedLevels);
    lcd.print(" "); lcd.print(rps, 1); lcd.print("RPS");
    lcd.setCursor(0, 1);
    lcd.print("a="); lcd.print(accelRevS2, 0); lcd.print(" r/s2");

    profileMove(&motor, rampRevs, CRUISE_REVS, rampRevs, rps);
    delay(300);
    moveToHome(&motor, 15.0f);
    delay(300);

    if (i < numSpeedLevels - 1) {
      Serial.println("  >> Press for next level\n");
      lcd.clear(); lcd.print("Press for next");
      waitForButton();
    }
  }

  Serial.println("\n=== MAX SPEED TEST COMPLETE ===\n");
  lcd.clear(); lcd.print("Done!");
  lcd.setCursor(0, 1); lcd.print("Press to repeat");
}
