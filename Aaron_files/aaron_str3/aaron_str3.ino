// STR3 Stepper Driver Control
// Direction: D9, Step: D10, Button: D22

#include "TrapezoidalMove.h"

// Pin definitions
const int DIR_PIN = 9;
const int STEP_PIN = 10;
const int BUTTON_PIN = 22;

const int STEPS_PER_REV = 200;

void setup() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.begin(115200);
  Serial.println("STR3 Stepper Ready");
  
  // Wait for button to be released before starting
  while (digitalRead(BUTTON_PIN) == LOW) {
    delay(10);
  }
}

void loop() {
  // Wait for button press (active LOW with pullup)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      // Example: Trapezoidal move
      trapezoidalMove(54.8, 15, 10.0); // revolutions, maxRPM, totalTime(seconds)
      trapezoidalMove(-54.8, 15, 30.0);
      // Wait for button release
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
}
