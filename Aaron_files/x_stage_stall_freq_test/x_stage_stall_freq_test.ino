// X-Axis Stage Stall Frequency Test — THK KR33 + STR3
// Dir=D51, Step=D53, Button=D22, Limits D2(End)/D3(Home), LCD: 7,8,4,5,6,11
//
// Tests which step frequencies cause stall during constant-velocity cruise.
// Each level: ramps up to target RPS at a fixed safe acceleration, then cruises.
// The ramp is gentle (SAFE_ACCEL_REV_S2) so accel is never the cause of stall.
// A stall during CRUISE = true frequency/resonance stall.

#include "TrapezoidalMove.h"

const int BUTTON_PIN = 22;

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

// Fixed ramp acceleration — well below stall threshold
const float SAFE_ACCEL_REV_S2 = 50.0f;
// Cruise distance — long enough to clearly observe a stall
const float CRUISE_REVS = 5.0f;

// Speed levels in RPS — edit these to target your resonance zones
float speedLevels[] = {
5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f, 33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f, 41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f, 48.0f, 49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f, 57.0f, 58.0f, 59.0f, 60.0f, 61.0f, 62.0f, 63.0f, 64.0f, 65.0f, 66.0f, 67.0f, 68.0f, 69.0f, 70.0f, 71.0f, 72.0f, 73.0f, 74.0f, 75.0f, 76.0f, 77.0f, 78.0f, 79.0f, 80.0f, 81.0f, 82.0f, 83.0f, 84.0f, 85.0f, 86.0f, 87.0f, 88.0f, 89.0f, 90.0f, 91.0f, 92.0f, 93.0f, 94.0f, 95.0f, 96.0f, 97.0f, 98.0f, 99.0f, 100.0f
 
};
const int numLevels = sizeof(speedLevels) / sizeof(speedLevels[0]);

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
  lcd.print("Stall Freq Test");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  motorInit(&motor, 1, true, 51, 53, 3200, false, &lcd, 2, 3, 6.0f, -1, 100, 20.0f);
  attachLimitInterrupts(&motor);

  calibrateAxis(&motor, 1.5f);
  moveToHome(&motor, 5.0f);

  Serial.println("=== X STAGE STALL FREQUENCY TEST ===");
  Serial.print("Accel ramp: "); Serial.print(SAFE_ACCEL_REV_S2); Serial.println(" rev/s² (constant)");
  Serial.print("Cruise: "); Serial.print(CRUISE_REVS); Serial.println(" rev");
  Serial.print("Levels: "); Serial.print(numLevels);
  Serial.print(" ("); Serial.print(speedLevels[0], 2);
  Serial.print(" – "); Serial.print(speedLevels[numLevels - 1], 2);
  Serial.println(" RPS)");
  Serial.println("Press button to begin.\n");
  lcd.clear();
  lcd.print("Press to start");
}

void loop() {
  updateLCD(&motor);
  waitForButton();

  Serial.println("\n=== STALL FREQ TEST START ===\n");

  for (int i = 0; i < numLevels; i++) {
    float rps      = speedLevels[i];
    long  hz       = (long)(rps * motor.stepsPerRev);
    float rampRevs = (rps * rps) / (2.0f * SAFE_ACCEL_REV_S2);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numLevels); Serial.print("]  ");
    Serial.print(rps, 2); Serial.print(" RPS  ("); Serial.print(hz); Serial.print(" Hz)");
    Serial.print("  ramp="); Serial.print(rampRevs, 3); Serial.print(" rev  — moving...");

    lcd.clear();
    lcd.print(i + 1); lcd.print("/"); lcd.print(numLevels);
    lcd.print(" "); lcd.print(rps, 2); lcd.print(" RPS");
    lcd.setCursor(0, 1);
    lcd.print(hz); lcd.print(" Hz");

    profileMove(&motor, rampRevs, CRUISE_REVS, rampRevs, rps);
    delay(200);
    moveToHome(&motor, 5.0f);  // creep to limit — works even if motor stalled and position is wrong
    delay(200);

    Serial.println("  done.");

    if (i < numLevels - 1) {
      Serial.println("  >> Record result. Press for next.\n");
      lcd.clear();
      lcd.print("Record result");
      lcd.setCursor(0, 1);
      lcd.print("Press for next");
      waitForButton();
    }
  }

  Serial.println("\n=== STALL FREQ TEST COMPLETE ===\n");
  lcd.clear();
  lcd.print("Done!");
  lcd.setCursor(0, 1);
  lcd.print("Press to repeat");
}
