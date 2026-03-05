// L298N H-Bridge Stepper Demo
// Motor coil A: OUT1/OUT2 | Motor coil B: OUT3/OUT4
// IN1=D7, IN2=D6, IN3=D5, IN4=D4 | ENA=D10, ENB=D9
//
// Duty cycle: 20% of 255 (matches original current-limiting target)
// Using 20% out of maximum ~22% for 38 V supply on L298N rated to 25 W

#include "lib/driver/stepper/l298n.h"
#include "lib/motor/rotational_motor.h"

//                  in1  in2  in3  in4  ena  enb  spr  duty
L298N           driver(7,   6,   5,   4,   10,  9,   200, (uint8_t)(0.20f * 255));  // in1, in2, in3, in4, enaPin, enbPin, stepsPerRev, dutyCycle
RotationalMotor motor;

void setup() {
  motor.init(1, &driver);  // id, driver
  driver.enable();

  motor.spinRevs( 4.0f, 100.0f / 60.0f);  // revolutions, rps  (800 steps @ 100 RPM)
  motor.spinRevs(-2.0f,  60.0f / 60.0f);  // revolutions, rps  (400 steps back @ 60 RPM)

  // ── Trapezoidal alternatives ───────────────────────────────────────────────
  //motor.autoTrapMove( 4.0f, 1.5f, 3.0f);   // revolutions, maxRPS, totalTime
  //motor.autoTrapMove(-2.0f, 1.0f, 3.0f);   // revolutions, maxRPS, totalTime

  //motor.manualTrapMove( 0.5f,  3.0f,  0.5f, 1.5f);  // accelRevs, cruiseRevs, decelRevs, cruiseRPS
  //motor.manualTrapMove(-0.5f, -3.0f, -0.5f, 1.0f);  // accelRevs, cruiseRevs, decelRevs, cruiseRPS

  driver.disable();
}

void loop() {}
