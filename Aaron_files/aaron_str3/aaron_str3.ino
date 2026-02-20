// STR3 Stepper Driver Control
// Direction: D9, Step: D10, Button: D22

#include "TrapezoidalMove.h"

// RS=7, EN=8, D4=4, D5=5, D6=6, D7=11
LiquidCrystal lcd(7, 8, 4, 5, 6, 11);

// Pin definitions
const int DIR_PIN = 9;
const int STEP_PIN = 10;
const int BUTTON_PIN = 22;
const int SENSOR_PIN_1 = 2; // End limit
const int SENSOR_PIN_2 = 3; // Home limit

const int STEPS_PER_REV = 200;

long  motorPosition  = 0;
long  endPosition    = 0;
long  axisLength     = 0;
float currentSpeedRPS = 0.0;

void setup() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SENSOR_PIN_1, INPUT);
  pinMode(SENSOR_PIN_2, INPUT);
  
  Serial.begin(115200);
  Serial.println("STR3 Stepper Ready");

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("STR3 Stepper");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");

  calibrateAxis(0.5);
  updateLCD();
  moveToHome(15);
}

void loop() {
  updateLCD();
  Serial.println("Waiting for button press...");
  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(10);
    updateLCD();
  }
  delay(50); // debounce
  if (digitalRead(BUTTON_PIN) == LOW) {
    // accelRevs, cruiseRevs, decelRevs, cruiseRPS
    profileMove(2.0, 40.0, 2.0, 15);
    profileMove(-2.0, -10.0, -2.0, 15);
    profileMove(2.0, 50.0, 2.0, 10);
    moveToHome(15);
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }
  }
}
