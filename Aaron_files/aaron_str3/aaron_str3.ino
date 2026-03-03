// STR3 Stepper Driver Control — X-Axis (THK KR33)
// Direction: D9, Step: D10, Button: D22, Limits: D2(End), D3(Home)
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "TrapezoidalMove.h"

const int BUTTON_PIN = 22;

// ── Tach Out interrupt counter ────────────────────────────────────────────
#define TACH_PIN            21   // Must be interrupt-capable (18/19/20/21 on Mega)
#define TACH_PULSES_PER_REV 100  // Match driver setting

volatile uint32_t tachCount = 0;

void onTachPulse() {
  tachCount++;
}

// Safely read total revolutions from tach (noInterrupts guard for 32-bit on 8-bit AVR)
float getTachRevolutions() {
  noInterrupts();
  uint32_t count = tachCount;
  interrupts();
  return count / (float)TACH_PULSES_PER_REV;
}

// Measure instantaneous RPS over a 50 ms sample window
float getTachRPS() {
  noInterrupts();
  uint32_t c0 = tachCount;
  interrupts();
  unsigned long t0 = micros();
  delayMicroseconds(50000);
  noInterrupts();
  uint32_t c1 = tachCount;
  interrupts();
  float elapsed = (micros() - t0) / 1e6;
  return ((c1 - c0) / (float)TACH_PULSES_PER_REV) / elapsed;
}
// ─────────────────────────────────────────────────────────────────────────

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;//linear stage
MotorConfig motor2;//zaxis
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("X-Axis");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  // motorInit(config, id, hasLimits, dirPin, stepPin,
  //           stepsPerRev, invertDir, lcd, limitEndPin, limitHomePin)
  motorInit(&motor, 1, true, 51, 53, 200, true, &lcd, 2, 3);
  //motorInit(&motor2, 2, false, 24, 25, 3200, false, &lcd);
  homeAxis(&motor, 1);
  //calibrateAxis(&motor, 0.5);
  //moveToHome(&motor, 5);
  //moveToHome(&motor, 10);

  pinMode(TACH_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TACH_PIN), onTachPulse, FALLING);
}

void loop() {
  updateLCD(&motor);

  // Wait for button release, then press, then release
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); }
  delay(50);
  //trapezoidalMove(&motor, 10, 5, 10);  distance, max speed, total time

  tachCount = 0;
  profileMove(&motor,  4.0,  30.0,  4.0, 3); //revs for accel, cruise, decel, speed in RPS
  Serial.print("Move 1 tach: "); Serial.print(getTachRevolutions(), 3); Serial.println(" rev");

  tachCount = 0;
  profileMove(&motor, -4.0, -30.0, -4.0, 3);
  Serial.print("Move 2 tach: "); Serial.print(getTachRevolutions(), 3); Serial.println(" rev");

  tachCount = 0;
  profileMove(&motor,  2.0,  50.0,  2.0, 3);
  Serial.print("Move 3 tach: "); Serial.print(getTachRevolutions(), 3); Serial.println(" rev");

  tachCount = 0;
  profileMove(&motor, -10.0,  0.0, -20.0, 3);
  Serial.print("Move 4 tach: "); Serial.print(getTachRevolutions(), 3); Serial.println(" rev");

  tachCount = 0;
  moveToHome(&motor, 3);//rps
  Serial.print("Home tach:   "); Serial.print(getTachRevolutions(), 3); Serial.println(" rev");
}
