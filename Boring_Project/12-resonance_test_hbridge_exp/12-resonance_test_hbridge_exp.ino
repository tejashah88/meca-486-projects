// H-Bridge Stepper Resonance Sweep — automated trap move sweep over a frequency range
// Motor coil A: OUT1/OUT2 | Motor coil B: OUT3/OUT4
// IN1=D49, IN2=D48, IN3=D47, IN4=D46 | ENA=D10, ENB=D9

#include "lib/driver/stepper/l298n.h"

//                     in1  in2  in3  in4  ena  enb  spr  duty
L298N driver(49, 48, 47, 46,  10,   9, 400, (uint8_t)(0.20f * 255));

// ── CONFIGURATION ──────────────────────────────────────────────────────────────
const float ACCEL_REV_S2  = 0.5f;    // constant acceleration (rev/s²)
const float CRUISE_SEC    = 2.0f;    // time to hold max speed before decelerating (s)
const float MIN_FREQ_HZ   = 20.0f;  // sweep start (step frequency = RPS * spr)
const float MAX_FREQ_HZ   = 1000.0f;// sweep end
const float FREQ_STEP_HZ  = 10.0f;  // increment between test points
const int   PAUSE_MS      = 500;    // pause between trap moves
// ──────────────────────────────────────────────────────────────────────────────

// Blocking trapezoidal move: accel → cruise → decel.
// Direction must be set via driver.setDirection() before calling.
// At step index i (1-based): speed = sqrt(2 * a * i / spr)  [from v² = 2a·s]
// accelSteps = maxRPS² / (2 * a) * spr
// cruiseSteps = cruiseSec * maxRPS * spr
void runTrap(float maxRPS, float accelRevSec2, float cruiseSec) {
  int spr = driver.stepsPerRev();
  int accelSteps  = max(1, (int)(maxRPS * maxRPS * spr / (2.0f * accelRevSec2)));
  int cruiseSteps = (int)(cruiseSec * maxRPS * spr);
  unsigned long cruisePeriod = (unsigned long)(1000000.0f / (maxRPS * spr));

  // Accel
  for (int i = 1; i <= accelSteps; i++) {
    float rps = sqrt(2.0f * accelRevSec2 * (float)i / spr);
    if (rps > maxRPS) rps = maxRPS;
    driver.step((unsigned long)(1000000.0f / (rps * spr)));
  }
  // Cruise
  for (int i = 0; i < cruiseSteps; i++) {
    driver.step(cruisePeriod);
  }
  // Decel (symmetric reverse of accel)
  for (int i = accelSteps; i >= 1; i--) {
    float rps = sqrt(2.0f * accelRevSec2 * (float)i / spr);
    if (rps > maxRPS) rps = maxRPS;
    driver.step((unsigned long)(1000000.0f / (rps * spr)));
  }
}

void setup() {
  Serial.begin(115200);

  driver.init();
  driver.setDirection(true);
  driver.enable();
  delay(500);  // let rotor align to phase 0

  Serial.println("=== H-BRIDGE STEPPER RESONANCE SWEEP ===");
  Serial.print("Accel: "); Serial.print(ACCEL_REV_S2, 1); Serial.print(" rev/s²  Cruise: "); Serial.print(CRUISE_SEC, 1); Serial.println(" s");
  Serial.print("Range: "); Serial.print(MIN_FREQ_HZ, 0);
  Serial.print(" – "); Serial.print(MAX_FREQ_HZ, 0);
  Serial.print(" Hz  step "); Serial.print(FREQ_STEP_HZ, 0); Serial.println(" Hz");
  Serial.println("-----------------------------------------");

  for (float hz = MIN_FREQ_HZ; hz <= MAX_FREQ_HZ + 0.5f; hz += FREQ_STEP_HZ) {
    float maxRPS = hz / driver.stepsPerRev();

    Serial.print("Hz: "); Serial.print(hz, 0);
    Serial.print("  RPS: "); Serial.println(maxRPS, 4);

    driver.setDirection(true);
    runTrap(maxRPS, ACCEL_REV_S2, CRUISE_SEC);
    delay(PAUSE_MS);
  }

  driver.disable();
  Serial.println("=== SWEEP COMPLETE ===");
}

void loop() {}
