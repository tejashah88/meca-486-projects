// Stall Test — STR3 Stepper on THK KR33
// Direction: D9, Step: D10, Button: D22, Limits: D2(End), D3(Home)
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "TrapezoidalMove.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

const float CRUISE_RPS  = 10.0;
const float CRUISE_REVS = 5.0;

float ramps[]  = { 3.0, 2.0, 1.5, 1.0, 0.75, 0.5, 0.35, 0.25, 0.15, 0.10, 0.05 };
int   numTests = sizeof(ramps) / sizeof(ramps[0]);

void waitForButtonPress() {
  while (digitalRead(motor.buttonPin) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(motor.buttonPin) == HIGH) { delay(10); updateLCD(&motor); }
  while (digitalRead(motor.buttonPin) == LOW)  { delay(10); }
  delay(50);
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Stall Test");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  motorInit(&motor, 1, true, 9, 10, 22, 200, &lcd, 2, 3);

  calibrateAxis(&motor, 0.5);
  homeAxis(&motor, 0.5);

  Serial.println("=== STALL TEST READY ===");
  Serial.print("Cruise: "); Serial.print(CRUISE_RPS); Serial.println(" RPS");
  lcd.clear(); lcd.print("Press to begin");
}

void loop() {
  waitForButtonPress();
  Serial.println("=== STALL TEST START ===\n");

  for (int i = 0; i < numTests; i++) {
    float ramp        = ramps[i];
    int   aSteps      = (int)(ramp * motor.stepsPerRev);
    float cruiseSpeed = CRUISE_RPS * motor.stepsPerRev;
    float accelRevS2  = (cruiseSpeed * cruiseSpeed) / (2.0 * aSteps * motor.stepsPerRev);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numTests); Serial.print("]  ");
    Serial.print("Ramp="); Serial.print(ramp, 2); Serial.print(" rev    ");
    Serial.print("Accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Level "); lcd.print(i + 1); lcd.print("/"); lcd.print(numTests);
    lcd.setCursor(0, 1); lcd.print(accelRevS2, 0); lcd.print(" rev/s^2");

    profileMove(&motor, ramp, CRUISE_REVS, ramp, CRUISE_RPS);
    delay(300);
    homeAxis(&motor, 2.0);
    delay(300);

    if (i < numTests - 1) {
      Serial.println("  >> Press for next level\n");
      lcd.clear(); lcd.print("Press for next");
      waitForButtonPress();
    }
  }

  Serial.println("=== STALL TEST COMPLETE ===");
  lcd.clear(); lcd.print("Done!"); lcd.setCursor(0, 1); lcd.print("Press to repeat");
}
