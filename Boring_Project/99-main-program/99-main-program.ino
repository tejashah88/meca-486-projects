// STR3 Stepper Driver Control
// Motor 1 (X-Axis, THK KR33): Dir=D51, Step=D53, Limits=D2(End)/D3(Home)
// Motor 2 (Z-Axis, no limits): Dir=D24, Step=D25
// Button: D22 | LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "lib/driver/stepper/str3.h"
#include "lib/motor/linear_motor.h"
#include "lib/motor/rotational_motor.h"
#include "lib/driver/lcd/lcd.h"
#include "lib/control/display/display.h"

const int BUTTON_PIN = 22;

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);  // RS, EN, D4, D5, D6, D7

STR3 xDriver(51, 53, 200);   // dirPin, stepPin, stepsPerRev
STR3 zDriver(24, 25, 3200);  // dirPin, stepPin, stepsPerRev

LinearMotor     xMotor;   // X-axis linear stage
RotationalMotor zMotor;   // Z-axis rotational stage

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  xMotor.init(1, &xDriver, 2, 3, 6.0f, 15.0f);  // id, driver, limitEndPin, limitHomePin, mmPerRev, maxRPS
  zMotor.init(2, &zDriver);                       // id, driver

  xMotor.enableLimits();
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); Display::renderMotorInfo(xMotor); }
  while (digitalRead(BUTTON_PIN) == LOW)   { delay(10); Display::renderMotorInfo(xMotor); }
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); }
  delay(50);
  //xMotor.findHome(1);  // slowRPS
  LCD::init(&lcd);
  LCD::clear();
  LCD::print("X-Axis");
  LCD::setCursor(0, 1);
  LCD::print("Calibrating...");
  xMotor.calibrate(1.5);   // slowRPS
  xMotor.goHome(20);       // cruiseRPS

  //xMotor.goHome(10);
}

void loop() {
  Display::renderMotorInfo(xMotor);

  // Wait for button release, then press, then release
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); Display::renderMotorInfo(xMotor); }
  while (digitalRead(BUTTON_PIN) == LOW)   { delay(10); Display::renderMotorInfo(xMotor); }
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); }
  delay(50);

  // ── Motor 1 (X-axis) ──────────────────────────────────────────────────
  xMotor.autoTrapMove(10, 5, 2);                  // revolutions, maxRPS, totalTime
  delay(500);
  zMotor.manualTrapMove( 2,  10,  2, 10);         // accelRevs, cruiseRevs, decelRevs, cruiseRPS
  zMotor.manualTrapMove(-2, -10, -2, 10);         // accelRevs, cruiseRevs, decelRevs, cruiseRPS
  delay(500);
  xMotor.autoTrapMove(10, 5, 2);                  // revolutions, maxRPS, totalTime
  delay(500);
  zMotor.manualTrapMove( 2,  10,  2, 10);         // accelRevs, cruiseRevs, decelRevs, cruiseRPS
  zMotor.manualTrapMove(-2, -10, -2, 10);         // accelRevs, cruiseRevs, decelRevs, cruiseRPS
  delay(500);
  xMotor.autoTrapMove(10, 5, 2);                  // revolutions, maxRPS, totalTime
  delay(500);
  zMotor.manualTrapMove( 2,  10,  2, 10);         // accelRevs, cruiseRevs, decelRevs, cruiseRPS
  zMotor.manualTrapMove(-2, -10, -2, 10);         // accelRevs, cruiseRevs, decelRevs, cruiseRPS
  delay(500);

  xMotor.goHome(15);                              // cruiseRPS

  // ── Motor 2 (Z-axis) ──────────────────────────────────────────────────

  //zMotor.autoTrapMove(10, 5, 10);               // revolutions, maxRPS, totalTime
}
