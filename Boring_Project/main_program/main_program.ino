// STR3 Stepper Driver Control
// Motor 1 (X-Axis, THK KR33): Dir=D51, Step=D53, Limits=D2(End)/D3(Home)
// Motor 2 (Z-Axis, no limits): Dir=D24, Step=D25
// Button: D22 | LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "driver/stepper/str3.h"
#include "motor/linear_motor.h"
#include "motor/rotational_motor.h"
#include "driver/lcd/lcd.h"
#include "control/display/display.h"

const int BUTTON_PIN = 22;

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);

STR3 xDriver(51, 53, 200);  // dirPin, stepPin, stepsPerRev
STR3 zDriver(24, 25, 3200);

LinearMotor    motor;   // X-axis linear stage
RotationalMotor motor2; // Z-axis

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // init(id, driver, limitEndPin, limitHomePin, mmPerRev, maxRPS)
  motor.init(1, &xDriver, 2, 3, 6.0f, 15.0f);
  motor2.init(2, &zDriver);

  motor.enableLimits();
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); Display::renderMotorInfo(motor); }
  while (digitalRead(BUTTON_PIN) == LOW)   { delay(10); Display::renderMotorInfo(motor); }
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); }
  delay(50);
  //motor.findHome(1);
  LCD::init(&lcd);
  LCD::clear();
  LCD::print("X-Axis");
  LCD::setCursor(0, 1);
  LCD::print("Calibrating...");
  motor.calibrate(1.5);
  motor.goHome(20);

  //motor.goHome(10);
}

void loop() {
  Display::renderMotorInfo(motor);

  // Wait for button release, then press, then release
  while (digitalRead(BUTTON_PIN) == LOW)   { delay(10); Display::renderMotorInfo(motor); }
  while (digitalRead(BUTTON_PIN) == HIGH)  { delay(10); Display::renderMotorInfo(motor); }
  while (digitalRead(BUTTON_PIN) == LOW)   { delay(10); }
  delay(50);

  // ── Motor 1 (X-axis) ──────────────────────────────────────────────────
  motor.autoTrapMove(10, 5, 2);
  delay(500);
  motor2.manualTrapMove( 2,  10,  2, 10); // 1 rev = 6mm
  motor2.manualTrapMove(-2, -10, -2, 10);
  delay(500);
  motor.autoTrapMove(10, 5, 2);
  delay(500);
  motor2.manualTrapMove( 2,  10,  2, 10);
  motor2.manualTrapMove(-2, -10, -2, 10);
  delay(500);
  motor.autoTrapMove(10, 5, 2);
  delay(500);
  motor2.manualTrapMove( 2,  10,  2, 10);
  motor2.manualTrapMove(-2, -10, -2, 10);
  delay(500);

  motor.goHome(15);

  // ── Motor 2 (Z-axis) ──────────────────────────────────────────────────

  //motor2.autoTrapMove(10, 5, 10);   // distance, max speed, total time
}
