// STR3 Stepper Driver Control
// Direction: D9, Step: D10, Button: D22

#include "TrapezoidalMove.h"

// Pin definitions
const int DIR_PIN = 9;
const int STEP_PIN = 10;
const int BUTTON_PIN = 22;
const int SENSOR_PIN_1 = 2; // End limit
const int SENSOR_PIN_2 = 3; // Home limit

const int STEPS_PER_REV = 200;

void setup() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SENSOR_PIN_1, INPUT);
  pinMode(SENSOR_PIN_2, INPUT);
  
  Serial.begin(115200);
  Serial.println("STR3 Stepper Ready");

  homeAxis(0.5);
}

void loop() {
  Serial.println("Waiting for button press...");
  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(10);
  }
  delay(50); // debounce
  if (digitalRead(BUTTON_PIN) == LOW) {
    // accelRevs, cruiseRevs, decelRevs, cruiseRPS
    profileMove(2.0, 45.0, 2.0, 2);
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }
  }
}
