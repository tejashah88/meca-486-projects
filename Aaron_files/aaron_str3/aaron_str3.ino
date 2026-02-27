// STR3 Stepper Driver Control — X-Axis (THK KR33)
// Direction: D9, Step: D10, Button: D22, Limits: D2(End), D3(Home)
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "TrapezoidalMove.h"

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;
MotorConfig motor2;
void setup() {
  Serial.begin(115200);
  Serial.println("STR3 Stepper Ready");

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("STR3 Stepper");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  // motorInit(config, id, hasLimits, dirPin, stepPin, buttonPin,
  //           stepsPerRev, lcd, limitEndPin, limitHomePin)
  //motorInit(&motor, 1, true, 9, 10, 22, 200, &lcd, 2, 3);
  motor.limitStopRevs = 1.0;
  motorInit(&motor2, 2, false, 9, 10, 22, 3200, &lcd);
  //homeAxis(&motor, 0.5);
  //calibrateAxis(&motor, 0.5);
  //moveToHome(&motor, 15);
}

void loop() {
  updateLCD(&motor);

  // Wait for button release, then press, then release
  while (digitalRead(motor.buttonPin) == LOW)  { delay(10); updateLCD(&motor); }
  while (digitalRead(motor.buttonPin) == HIGH) { delay(10); updateLCD(&motor); }
  while (digitalRead(motor.buttonPin) == LOW)  { delay(10); }
  delay(50);

  profileMove(&motor2,  2.0,  40.0,  2.0, 15);
  profileMove(&motor2, -2.0, -10.0, -2.0, 15);
  profileMove(&motor2,  2.0,  50.0,  2.0, 10);
  profileMove(&motor2, -10.0,  0.0, -20.0, 10);
  //moveToHome(&motor, 15);
}
