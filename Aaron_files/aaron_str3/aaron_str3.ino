// STR3 Stepper Driver Control — X-Axis (THK KR33)
// Direction: D9, Step: D10, Button: D22, Limits: D2(End), D3(Home)
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "TrapezoidalMove.h"

const int BUTTON_PIN = 22;

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
  motorInit(&motor, 1, true, 25, 23, 200, false, &lcd, 2, 3);
  //motorInit(&motor2, 2, false, 24, 25, 3200, false, &lcd);
  homeAxis(&motor, 1);
  //calibrateAxis(&motor, 0.5);
  //moveToHome(&motor, 5);
  //moveToHome(&motor, 10);
}

void loop() {
  updateLCD(&motor);

  // Wait for button release, then press, then release
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); updateLCD(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); }
  delay(50);
  //trapezoidalMove(&motor, 10, 5, 10);  distance, max speed, total time
  profileMove(&motor,  4.0,  30.0,  4.0, 15); //revs for accel, cruise, decel, speed in RPS
  profileMove(&motor, -4.0, -30.0, -4.0, 5);
  profileMove(&motor,  2.0,  50.0,  2.0, 5);
  profileMove(&motor, -10.0,  0.0, -20.0, 5);
  moveToHome(&motor, 10);//rps
}
