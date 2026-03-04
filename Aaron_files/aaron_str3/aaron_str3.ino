// STR3 Stepper Driver Control
// Motor 1 (X-Axis, THK KR33): Dir=D51, Step=D53, Limits=D2(End)/D3(Home)
// Motor 2 (Z-Axis, no limits): Dir=D24, Step=D25
// Button: D22 | LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "TrapezoidalMove.h"

const int BUTTON_PIN = 22;

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;  // linear stage
MotorConfig motor2; // z-axis

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);



  // motorInit(config, id, hasLimits, dirPin, stepPin,
  //           stepsPerRev, invertDir, lcd, limitEndPin, limitHomePin,
  //           mmPerRev, tachPin=-1 (disabled), tachPulsesPerRev=100, maxRPS)
  motorInit(&motor, 1, true, 51, 53, 200, false, &lcd, 2, 3, 6.0f, -1, 100, 15.0f);
  // Z-axis: no limits, 3200 steps/rev (microstepping), adjust invertDir and maxRPS as needed
  motorInit(&motor2, 2, false, 24, 25, 3200, false, &lcd, -1, -1, 0.0f, -1, 100, 5.0f);

  attachLimitInterrupts(&motor);
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW) { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); }
  delay(50);
  //homeAxis(&motor, 1);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("X-Axis");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");
  calibrateAxis(&motor, 1.5);
  moveToHome(&motor, 20);


  //moveToHome(&motor, 10);
}

void loop() {
  updateLCD(&motor);

  // Wait for button release, then press, then release
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW) { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); }
  delay(50);

  // ── Motor 1 (X-axis) ──────────────────────────────────────────────────
  trapezoidalMove(&motor, 10, 5, 2);
  delay(500);
  profileMove(&motor2,  2,  10,  2, 10);//1 rev = 6mm
  profileMove(&motor2,  -2,  -10,  -2, 10);
   delay(500);
  trapezoidalMove(&motor, 10, 5, 2);
   delay(500);
  profileMove(&motor2,  2,  10,  2, 10);
  profileMove(&motor2,  -2,  -10,  -2, 10);
   delay(500);
  trapezoidalMove(&motor, 10, 5, 2);
  delay(500);
  profileMove(&motor2,  2,  10,  2, 10);
  profileMove(&motor2,  -2,  -10,  -2, 10);
  delay(500);

  moveToHome(&motor, 15);

  // ── Motor 2 (Z-axis) ──────────────────────────────────────────────────


  //trapezoidalMove(&motor2, 10, 5, 10);        // distance, max speed, total time
}
