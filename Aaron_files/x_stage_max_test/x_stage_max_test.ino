// X-Axis Linear Stage — Max Speed vs Voltage Test
// THK KR33 + STR3. Dir=D51, Step=D53, Button=D22, Limits D2(End)/D3(Home), LCD: 7,8,4,5,6,11
//
// Each speed level runs TWICE:
//   Run A → return home → "Switch voltage" prompt → Run B → return home → next level
// Record pass/stall for each voltage at each speed.
//
// Ramp = v²/(2*SAFE_ACCEL_REV_S2) so motor always reaches cruise speed.
// Cruise = CRUISE_REVS to confirm it sustains that speed.
// If ramp+cruise+ramp exceeds stage length, that level is skipped.
// Change SPR to match your driver microstep setting.

#include "TrapezoidalMove.h"

// ── CONFIG ────────────────────────────────────────────────────────────────
const int   BUTTON_PIN        = 22;
const int   SPR               = 1600;   // steps/rev — UPDATE to match driver microstep setting
const float SAFE_ACCEL_REV_S2 = 50.0f; // ramp accel — gentle enough to always reach cruise
const float CRUISE_REVS       = 5.0f;  // cruise distance to confirm motor sustains the speed
const float STAGE_MARGIN_REVS = 1.0f;  // clearance from each limit (rev)

const char* LABEL_A = "Voltage A";     // label printed for first run
const char* LABEL_B = "Voltage B";     // label printed for second run

// Speed sweep: 5 – 21 RPS in 1 RPS steps
float speedLevels[] = {
   5.0f,  6.0f,  7.0f,  8.0f,  9.0f, 10.0f,
  11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
  17.0f, 18.0f, 19.0f, 20.0f, 21.0f
};
const int numLevels = sizeof(speedLevels) / sizeof(speedLevels[0]);
// ─────────────────────────────────────────────────────────────────────────

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

void waitForButton() {
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); }
  delay(50);
}

void doRun(float rps, const char* label) {
  float rampRevs = (rps * rps) / (2.0f * SAFE_ACCEL_REV_S2);
  long  hz       = (long)(rps * motor.stepsPerRev);

  Serial.print("  ["); Serial.print(label); Serial.print("]  ");
  Serial.print(rps, 1); Serial.print(" RPS  "); Serial.print(hz); Serial.print(" Hz");
  Serial.print("  ramp="); Serial.print(rampRevs, 2); Serial.print(" rev  — moving...");

  lcd.clear();
  lcd.print(label);
  lcd.setCursor(0, 1);
  lcd.print(rps, 1); lcd.print("RPS "); lcd.print(hz); lcd.print("Hz");

  profileMove(&motor, rampRevs, CRUISE_REVS, rampRevs, rps);
  delay(300);
  homeAxis(&motor, 5.0f);
  delay(300);

  Serial.println("  done.");
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Speed/Volt Test");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  motorInit(&motor, 1, true, 51, 53, SPR, false, &lcd, 2, 3, 6.0f, -1, 100, 25.0f);
  attachLimitInterrupts(&motor);

  calibrateAxis(&motor, 1.5f);
  moveToHome(&motor, 5.0f);

  float travelRevs = ((float)motor.axisLength / motor.stepsPerRev) - 2.0f * STAGE_MARGIN_REVS;

  Serial.println("=== X STAGE MAX SPEED vs VOLTAGE TEST ===");
  Serial.print("SPR: "); Serial.print(SPR);
  Serial.print("  Stage travel: "); Serial.print(travelRevs, 2); Serial.println(" rev");
  Serial.print("Accel: "); Serial.print(SAFE_ACCEL_REV_S2); Serial.print(" rev/s²  Cruise: ");
  Serial.print(CRUISE_REVS); Serial.println(" rev");
  Serial.print("Levels: "); Serial.print(numLevels);
  Serial.print("  ("); Serial.print(speedLevels[0], 0);
  Serial.print(" – "); Serial.print(speedLevels[numLevels - 1], 0); Serial.println(" RPS)");
  Serial.println("Each level: Run A → switch voltage → Run B → press for next\n");

  lcd.clear();
  lcd.print("Set "); lcd.print(LABEL_A);
  lcd.setCursor(0, 1);
  lcd.print("Press to start");
}

void loop() {
  updateLCD(&motor);
  waitForButton();

  float travelRevs = ((float)motor.axisLength / motor.stepsPerRev) - 2.0f * STAGE_MARGIN_REVS;

  Serial.println("=== TEST START ===\n");

  for (int i = 0; i < numLevels; i++) {
    float rps      = speedLevels[i];
    float rampRevs = (rps * rps) / (2.0f * SAFE_ACCEL_REV_S2);
    float totalRevs = 2.0f * rampRevs + CRUISE_REVS;

    if (totalRevs > travelRevs) {
      Serial.print("── Level "); Serial.print(i + 1); Serial.print("  "); Serial.print(rps, 1);
      Serial.print(" RPS — SKIP (needs "); Serial.print(totalRevs, 1);
      Serial.print(" rev, have "); Serial.print(travelRevs, 1); Serial.println(" rev)");
      continue;
    }

    Serial.print("── Level "); Serial.print(i + 1); Serial.print("/"); Serial.print(numLevels);
    Serial.print("  "); Serial.print(rps, 1); Serial.print(" RPS  ramp=");
    Serial.print(rampRevs, 2); Serial.println(" rev ──");

    // Run A
    doRun(rps, LABEL_A);

    // Prompt voltage switch
    Serial.print("  >> Switch to "); Serial.print(LABEL_B); Serial.println(" — press when ready.");
    lcd.clear(); lcd.print("Switch to");
    lcd.setCursor(0, 1); lcd.print(LABEL_B);
    waitForButton();

    // Run B
    doRun(rps, LABEL_B);

    if (i < numLevels - 1) {
      Serial.print("  >> Switch back to "); Serial.print(LABEL_A); Serial.println(". Press for next speed.\n");
      lcd.clear(); lcd.print("Switch to");
      lcd.setCursor(0, 1); lcd.print(LABEL_A);
      waitForButton();
    }
  }

  Serial.println("\n=== TEST COMPLETE ===\n");
  lcd.clear(); lcd.print("Done!");
  lcd.setCursor(0, 1); lcd.print("Press to repeat");
}
