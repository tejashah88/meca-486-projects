#include "aaron_stepper.h"

// Hardware pin definitions
const int ENABLE_SIDE_A_PIN = 10;
const int ENABLE_SIDE_B_PIN = 9;
const int INPUT_PIN_1 = 7;
const int INPUT_PIN_2 = 6;
const int INPUT_PIN_3 = 5;
const int INPUT_PIN_4 = 4;

// Motor configuration
const int STEPS_PER_REV = 200;
const int INITIAL_RPM = 100;
// Using 20% out of maximum 29.4% duty cycle (30 V nominal) for current limiting
// Measured voltage from AMP PS430 is 38 V, thus maximum is 22% smth
// L298N can only handle maximum 25 W
const int ENABLE_DUTY_CYCLE_AMT = (int)(0.20 * 255);

// Create stepper object - all state is now managed inside the class
Aaron_Stepper stepper(ENABLE_SIDE_A_PIN, ENABLE_SIDE_B_PIN, 
                      INPUT_PIN_1, INPUT_PIN_2, INPUT_PIN_3, INPUT_PIN_4,
                      STEPS_PER_REV, INITIAL_RPM, ENABLE_DUTY_CYCLE_AMT);

void setup() {
  // Initialize the stepper (sets up pins and initial position)
  stepper.begin();
  
  delay(1000);
  
  // Move 800 steps forward
  stepper.moveSteps(800);
  
  // Change speed to 60 RPM
  stepper.setRPM(60);
  
  // Move 400 steps backward
  stepper.moveSteps(-400);
  
  // Disable motor
  stepper.disable();
}

void loop() {
  // Your control code here
}
