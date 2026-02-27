// Motor-Only Stall & Resonance Test — STR3 + Rotating Mass
// No limit switches. hasLimits=false disables all limit checking.
// SHORT press (< 1s) → Resonance test
// LONG press  (≥ 1s) → Stall test
// Direction: D9, Step: D10, Button: D22
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "TrapezoidalMove.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;

// ── CHANGE TO MATCH YOUR DRIVER ───────────────────────────────────────────
const int SPR = 200;  // steps/rev: 200,400,800,1000,1600,2000,3200...

const float MAX_RPS     = 1000000.0 / (2.0 * 5.0) / SPR;
const float CRUISE_RPS  = 10.0;
const float CRUISE_REVS = 5.0;
const float ROT_REVS    = 3.0;

float stallRamps[]   = { 3.0, 2.0, 1.5, 1.0, 0.75, 0.5, 0.35, 0.25, 0.15, 0.10, 0.05 };
int   numStallLevels = sizeof(stallRamps) / sizeof(stallRamps[0]);

const float ALL_SPEEDS[] = {
  0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90,
  1.00, 1.25, 1.50, 1.75, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00,
  6.00, 7.00, 8.00, 9.00, 10.0, 12.0, 15.0, 18.0, 20.0,
  25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 75.0, 100.0
};
const int TOTAL_SPEEDS = sizeof(ALL_SPEEDS) / sizeof(ALL_SPEEDS[0]);

// ── Helpers ───────────────────────────────────────────────────────────────

unsigned long measurePressDuration() {
  unsigned long t = millis();
  while (digitalRead(motor.buttonPin) == LOW) { delay(5); }
  delay(50);
  return millis() - t;
}

void waitForButtonPress() {
  while (digitalRead(motor.buttonPin) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(motor.buttonPin) == HIGH) { delay(10); updateLCD(&motor); }
  while (digitalRead(motor.buttonPin) == LOW)  { delay(10); }
  delay(50);
}

// ── Setup ─────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Motor-Only Test");
  lcd.setCursor(0, 1);
  lcd.print("SPR:"); lcd.print(SPR);

  // hasLimits=false — no limit pins needed, no homing, no calibration
  motorInit(&motor, 1, false, 9, 10, 22, SPR, &lcd);

  Serial.println("=== MOTOR-ONLY TEST ===");
  Serial.print("SPR="); Serial.print(SPR);
  Serial.print("  MaxRPS="); Serial.println(MAX_RPS, 1);
  Serial.println("SHORT press = Resonance | LONG press = Stall\n");

  delay(1000);
  lcd.setCursor(0, 1); lcd.print("S=Res  L=Stall");
}

// ── Resonance test ────────────────────────────────────────────────────────

void resonanceTest() {
  Serial.println("=== RESONANCE TEST START ===\n");

  int lvl = 0;
  for (int i = 0; i < TOTAL_SPEEDS; i++) {
    float rps    = ALL_SPEEDS[i];
    float stepHz = rps * SPR;

    if (rps > MAX_RPS) {
      Serial.print("[ SKIP ] "); Serial.print(rps, 2); Serial.println(" RPS");
      continue;
    }

    lvl++;
    Serial.print("[Lv"); Serial.print(lvl); Serial.print("]  ");
    Serial.print(rps, 2); Serial.print(" RPS  ");
    Serial.print(stepHz, 0); Serial.print(" Hz    Vibration: ___");

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(rps, 2); lcd.print(" RPS");
    lcd.setCursor(0, 1); lcd.print((int)stepHz); lcd.print("Hz Lv"); lcd.print(lvl);

    rotate(&motor,  ROT_REVS, rps);
    delay(150);
    rotate(&motor, -ROT_REVS, rps);
    delay(150);

    Serial.println();

    if (i < TOTAL_SPEEDS - 1 && ALL_SPEEDS[i + 1] <= MAX_RPS) {
      lcd.clear(); lcd.print("Note vibration");
      lcd.setCursor(0, 1); lcd.print("Press for next");
      waitForButtonPress();
    }
  }
  Serial.println("=== RESONANCE COMPLETE ===\n");
}

// ── Stall test ────────────────────────────────────────────────────────────

void stallTest() {
  Serial.println("=== STALL TEST START ===\n");
  float cruiseSpeed = CRUISE_RPS * motor.stepsPerRev;

  for (int i = 0; i < numStallLevels; i++) {
    float ramp       = stallRamps[i];
    int   aSteps     = (int)(ramp * motor.stepsPerRev);
    float accelRevS2 = (cruiseSpeed * cruiseSpeed) / (2.0 * aSteps * motor.stepsPerRev);

    Serial.print("["); Serial.print(i + 1); Serial.print("/"); Serial.print(numStallLevels); Serial.print("]  ");
    Serial.print("Ramp="); Serial.print(ramp, 2); Serial.print(" rev    ");
    Serial.print("Accel="); Serial.print(accelRevS2, 1); Serial.println(" rev/s²");

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Stall Lv"); lcd.print(i + 1);
    lcd.setCursor(0, 1); lcd.print(accelRevS2, 0); lcd.print(" rev/s^2");

    profileMove(&motor,  ramp,  CRUISE_REVS,  ramp, CRUISE_RPS);
    delay(300);
    profileMove(&motor, -ramp, -CRUISE_REVS, -ramp, CRUISE_RPS);
    delay(300);

    if (i < numStallLevels - 1) {
      Serial.println("  >> Press for next level\n");
      lcd.clear(); lcd.print("Pass/Fail?");
      lcd.setCursor(0, 1); lcd.print("Press for next");
      waitForButtonPress();
    }
  }
  Serial.println("=== STALL COMPLETE ===\n");
}

// ── Main loop ─────────────────────────────────────────────────────────────

void loop() {
  updateLCD(&motor);
  while (digitalRead(motor.buttonPin) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(motor.buttonPin) == HIGH) { delay(10); updateLCD(&motor); }

  if (measurePressDuration() >= 1000) {
    lcd.clear(); lcd.print("STALL TEST"); lcd.setCursor(0, 1); lcd.print("Starting...");
    delay(400);
    stallTest();
  } else {
    lcd.clear(); lcd.print("RESONANCE TEST"); lcd.setCursor(0, 1); lcd.print("Starting...");
    delay(400);
    resonanceTest();
  }

  lcd.clear(); lcd.print("Done!");
  lcd.setCursor(0, 1); lcd.print("S=Res  L=Stall");
}
