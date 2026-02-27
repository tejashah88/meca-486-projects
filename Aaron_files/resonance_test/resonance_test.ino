// Resonance Region Test — STR3 Stepper on THK KR33
// Direction: D9, Step: D10, Button: D22, Limits: D2(End), D3(Home)
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "TrapezoidalMove.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

// ── CHANGE TO MATCH YOUR DRIVER ───────────────────────────────────────────
const int SPR = 200;  // steps/rev: 200,400,800,1000,1600,2000,3200...

const float MAX_RPS     = 1000000.0 / (2.0 * 5.0) / SPR;
const float TRAVEL_REVS = 3.0;

const float ALL_SPEEDS[] = {
  0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90,
  1.00, 1.25, 1.50, 1.75, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00,
  6.00, 7.00, 8.00, 9.00, 10.0, 12.0, 15.0, 18.0, 20.0,
  25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 75.0, 100.0
};
const int TOTAL_SPEEDS = sizeof(ALL_SPEEDS) / sizeof(ALL_SPEEDS[0]);

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
  lcd.print("Resonance Test");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  motorInit(&motor, 1, true, 9, 10, 22, SPR, &lcd, 2, 3);

  calibrateAxis(&motor, 0.5);
  homeAxis(&motor, 0.5);

  int valid = 0;
  for (int i = 0; i < TOTAL_SPEEDS; i++) if (ALL_SPEEDS[i] <= MAX_RPS) valid++;

  Serial.println("=== RESONANCE TEST READY ===");
  Serial.print("SPR="); Serial.print(SPR);
  Serial.print("  MaxRPS="); Serial.print(MAX_RPS, 1);
  Serial.print("  Levels="); Serial.println(valid);
  Serial.println("Vibration: 0=None 1=Mild 2=Moderate 3=Strong X=Stall\n");

  lcd.clear(); lcd.print("SPR:"); lcd.print(SPR);
  lcd.setCursor(0, 1); lcd.print("Press to begin");
}

void loop() {
  waitForButtonPress();
  Serial.println("=== SWEEP START ===\n");

  int lvl = 0;
  for (int i = 0; i < TOTAL_SPEEDS; i++) {
    float rps    = ALL_SPEEDS[i];
    float stepHz = rps * SPR;

    if (rps > MAX_RPS) {
      Serial.print("[ SKIP ] "); Serial.print(rps, 2);
      Serial.print(" RPS ("); Serial.print(stepHz / 1000.0, 1); Serial.println(" kHz)");
      continue;
    }

    lvl++;
    Serial.print("[Lv"); Serial.print(lvl); Serial.print("]  ");
    Serial.print(rps, 2); Serial.print(" RPS  ");
    Serial.print(stepHz, 0); Serial.print(" Hz    Vibration: ___");

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(rps, 2); lcd.print(" RPS");
    lcd.setCursor(0, 1); lcd.print((int)stepHz); lcd.print("Hz Lv"); lcd.print(lvl);

    rotate(&motor,  TRAVEL_REVS, rps);
    delay(200);
    rotate(&motor, -TRAVEL_REVS, rps);
    delay(200);
    homeAxis(&motor, 2.0);
    delay(200);
    Serial.println();

    if (i < TOTAL_SPEEDS - 1 && ALL_SPEEDS[i + 1] <= MAX_RPS) {
      lcd.clear(); lcd.print("Note vibration");
      lcd.setCursor(0, 1); lcd.print("Press for next");
      waitForButtonPress();
    }
  }

  Serial.println("=== SWEEP COMPLETE ===\n");
  lcd.clear(); lcd.print("Done!");
  lcd.setCursor(0, 1); lcd.print("Press to repeat");
}
