// STR3 Stepper Driver Control
// Motor 1 (X-Axis, THK KR33): Dir=D51, Step=D53, Limits=D2(End)/D3(Home)
// Motor 2 (Z-Axis, no limits): Dir=D24, Step=D25
// Button: D22 | LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include "motors.h"

const int BUTTON_PIN = 22;

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);
MotorConfig motor;  // linear stage
MotorConfig motor2; // z-axis

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // STR3::init(config, id, hasLimits, dirPin, stepPin,
  //            stepsPerRev, invertDir, limitEndPin, limitHomePin, mmPerRev, maxRPS)
  STR3::init(&motor,  1, true,  51, 53, 200,  false, 2, 3, 6.0f, 15.0f);
  STR3::init(&motor2, 2, false, 24, 25, 3200, false);

  LimitSw::attach(&motor);
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); Display::updateMotor(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); Display::updateMotor(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); }
  delay(50);
  //Cal::homeAxis(&motor, 1);
  LCD::init(&lcd);
  LCD::clear();
  LCD::print("X-Axis");
  LCD::setCursor(0, 1);
  LCD::print("Calibrating...");
  Cal::calibrateAxis(&motor, 1.5);
  Cal::moveToHome(&motor, 20);

  //Cal::moveToHome(&motor, 10);
}

void loop() {
  Display::updateMotor(&motor);

  // Wait for button release, then press, then release
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); Display::updateMotor(&motor); }
  while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); Display::updateMotor(&motor); }
  while (digitalRead(BUTTON_PIN) == LOW)  { delay(10); }
  delay(50);

  // ── Motor 1 (X-axis) ──────────────────────────────────────────────────
  MotionProf::trapezoidalMove(&motor, 10, 5, 2);
  delay(500);
  MotionProf::profileMove(&motor2,  2,  10,  2, 10); // 1 rev = 6mm
  MotionProf::profileMove(&motor2, -2, -10, -2, 10);
  delay(500);
  MotionProf::trapezoidalMove(&motor, 10, 5, 2);
  delay(500);
  MotionProf::profileMove(&motor2,  2,  10,  2, 10);
  MotionProf::profileMove(&motor2, -2, -10, -2, 10);
  delay(500);
  MotionProf::trapezoidalMove(&motor, 10, 5, 2);
  delay(500);
  MotionProf::profileMove(&motor2,  2,  10,  2, 10);
  MotionProf::profileMove(&motor2, -2, -10, -2, 10);
  delay(500);

  Cal::moveToHome(&motor, 15);

  // ── Motor 2 (Z-axis) ──────────────────────────────────────────────────

  //MotionProf::trapezoidalMove(&motor2, 10, 5, 10);   // distance, max speed, total time
}
