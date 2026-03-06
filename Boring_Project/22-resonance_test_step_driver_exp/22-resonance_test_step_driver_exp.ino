// STR3 Stepper Resonance Sweep — automated trap move sweep over a frequency range
// STR3: Dir=D51, Step=D53, 200 spr

#include "lib/driver/stepper/str3.h"
#include "lib/motor/rotational_motor.h"

STR3           driver(51, 53, 200);  // dirPin, stepPin, stepsPerRev
RotationalMotor motor;

// ── CONFIGURATION ──────────────────────────────────────────────────────────────
const float         ACCEL_REV_S2     = 1.0f;   // constant acceleration (rev/s²)
const float         DECEL_REV_S2     = 1.0f;   // constant deceleration (rev/s²)
const float         CRUISE_SEC       = 5.0f;   // time to hold max speed before decelerating (s)
const float         MIN_FREQ_HZ      = 1.0f;   // sweep start (step frequency = RPS * spr)
const float         MAX_FREQ_HZ      = 50.0f;  // sweep end
const float         FREQ_STEP_HZ     = 1.0f;   // increment between test points
const int           PAUSE_MS         = 500;    // pause between trap moves
const unsigned long COMP_THRESHOLD_US = 1750;  // use micros() compensation below this period
                                               // (~10× AVR sqrt overhead of ~175 µs)
// ──────────────────────────────────────────────────────────────────────────────

// Blocking trapezoidal move: accel → cruise → decel.
// Direction must be set via driver.setDirection() before calling.
// period = 1e6 / sqrt(2·a·n·spr)  [from v²=2a·s, T=1/(v·spr)]
// micros() compensation absorbs sqrt overhead into the accel step delay.
// Decel uses no compensation: sqrt overhead adds to each period (monotonic, no slip).
// accelSteps = maxRPS² / (2·accel) · spr
// decelSteps = maxRPS² / (2·decel) · spr
void runTrap(float maxRPS, float accelRevSec2, float decelRevSec2, float cruiseSec) {
  int spr = driver.stepsPerRev();
  long accelSteps  = max(1L, (long)(maxRPS * maxRPS * spr / (2.0f * accelRevSec2)));
  long decelSteps  = max(1L, (long)(maxRPS * maxRPS * spr / (2.0f * decelRevSec2)));
  long cruiseSteps = (long)(cruiseSec * maxRPS * spr);
  unsigned long cruisePeriod = (unsigned long)(1000000.0f / (maxRPS * spr));

  // Accel — micros() compensation used when period < COMP_THRESHOLD_US (sqrt overhead matters).
  // At long periods (low speed) the ~175 µs overhead is negligible; step directly.
  unsigned long prevUs = micros();
  for (long n = 1; n <= accelSteps; n++) {
    unsigned long period = (unsigned long)(1000000.0f / sqrt(2.0f * accelRevSec2 * (float)n * spr));
    if (period < cruisePeriod) period = cruisePeriod;
    if (period < COMP_THRESHOLD_US) {
      unsigned long elapsed = micros() - prevUs;
      unsigned long remain  = (elapsed < period) ? (period - elapsed) : 2UL;
      driver.step(remain);
    } else {
      driver.step(period);
    }
    prevUs = micros();
  }

  // Cruise
  for (long i = 0; i < cruiseSteps; i++) {
    driver.step(cruisePeriod);
  }

  // Decel — same threshold logic; prevUs reset after cruise to avoid stale timestamp.
  prevUs = micros();
  for (long n = decelSteps; n >= 1; n--) {
    unsigned long period = (unsigned long)(1000000.0f / sqrt(2.0f * decelRevSec2 * (float)n * spr));
    if (period < cruisePeriod) period = cruisePeriod;
    if (period < COMP_THRESHOLD_US) {
      unsigned long elapsed = micros() - prevUs;
      unsigned long remain  = (elapsed < period) ? (period - elapsed) : 2UL;
      driver.step(remain);
    } else {
      driver.step(period);
    }
    prevUs = micros();
  }
}

void setup() {
  Serial.begin(115200);

  driver.init();
  motor.init(1, &driver);

  // // ── Motor sanity check — uncomment to verify wiring before running sweep ────
  // // Spins 3 revs forward @ 1 RPS, pauses, then 3 revs backward @ 1 RPS.
  // motor.spinRevs( 3.0f, 1.0f);
  // delay(1000);
  // motor.spinRevs(-3.0f, 1.0f);
  // delay(1000);
  // ────────────────────────────────────────────────────────────────────────────

  Serial.println("=== STR3 STEPPER RESONANCE SWEEP ===");
  Serial.print("Accel: "); Serial.print(ACCEL_REV_S2, 1); Serial.print(" rev/s²  Cruise: "); Serial.print(CRUISE_SEC, 1); Serial.println(" s");
  Serial.print("Range: "); Serial.print(MIN_FREQ_HZ, 0);
  Serial.print(" – "); Serial.print(MAX_FREQ_HZ, 0);
  Serial.print(" Hz  step "); Serial.print(FREQ_STEP_HZ, 0); Serial.println(" Hz");
  Serial.println("------------------------------------");

  for (float hz = MIN_FREQ_HZ; hz <= MAX_FREQ_HZ + 0.5f; hz += FREQ_STEP_HZ) {
    float maxRPS = hz / driver.stepsPerRev();

    Serial.print("Hz: "); Serial.print(hz, 0);
    Serial.print("  RPS: "); Serial.println(maxRPS, 4);

    driver.setDirection(true);
    runTrap(maxRPS, ACCEL_REV_S2, DECEL_REV_S2, CRUISE_SEC);
    delay(PAUSE_MS);
  }

  Serial.println("=== SWEEP COMPLETE ===");
}

void loop() {}
