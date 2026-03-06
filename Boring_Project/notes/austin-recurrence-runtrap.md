# Austin Recurrence — runTrap() Implementation

Replaces per-step `sqrt()` (~175 µs on AVR) with an iterative recurrence (~5 µs/step).
One `sqrt` call seeds `T_1`; each subsequent period is computed from the previous via
integer multiply/divide only. Removes the ~5–6 kHz frequency ceiling imposed by sqrt overhead.

## Background

From kinematics: `T_n = 1e6 / sqrt(2·a·n·spr)`

Ratio of consecutive periods: `T_{n+1} / T_n = sqrt(n/(n+1))`

Austin's first-order Taylor approximation (Circuit Cellar, 2004):

```
T_{n+1} = T_n - 2·T_n / (4n+1)        // accel (period decreasing)
T_{n-1} = T_n + 2·T_n / (4n-5)        // decel (period increasing, inverse recurrence)
```

Accuracy: ~1% error for first ~5 steps; excellent from step 10 onwards.
Error at low n is irrelevant — motor starts near zero speed regardless.

## Integer precision issue (why the first attempt failed)

Storing period in **microseconds** causes `2·T/(4n+1)` to round to zero for large step
counts before the period reaches cruise speed. At n=20833 (5 kHz, a=3 rev/s²):

```
2 × 200 µs / 83333 = 0   → recurrence stalls, motor plateaus early
```

Fix: store period in **nanoseconds** (×1000). Same n:
```
2 × 200,000 ns / 83333 = 4 ns   → recurrence still advances ✓
```

`unsigned long` (32-bit) holds up to ~4.3 billion ns. T_1 for typical configs is
well under 500 million ns. Safe for all practical accel/spr combinations.

## Corrected implementation (Austin ns + micros() compensation)

```cpp
// Blocking trapezoidal move: accel → cruise → decel.
// Direction must be set via driver.setDirection() before calling.
// Period stored in nanoseconds (×1000 vs µs) so Austin recurrence stays precise at
// large step counts where 2·T/(4n+1) would round to 0 in integer microseconds.
// micros() compensation absorbs per-step computation overhead into the step delay.
// accelSteps = maxRPS² / (2·a) * spr
// cruiseSteps = cruiseSec * maxRPS * spr
void runTrap(float maxRPS, float accelRevSec2, float cruiseSec) {
  int spr = driver.stepsPerRev();
  long accelSteps    = max(1L, (long)(maxRPS * maxRPS * spr / (2.0f * accelRevSec2)));
  long cruiseSteps   = (long)(cruiseSec * maxRPS * spr);
  unsigned long cruisePeriod = (unsigned long)(1000000.0f / (maxRPS * spr));  // µs

  // Seed T_1 = 1e9 / sqrt(2·a·spr) — one sqrt call per trap move
  unsigned long stepNs   = (unsigned long)(1000000000.0f / sqrt(2.0f * accelRevSec2 * (float)spr));
  unsigned long cruiseNs = (unsigned long)(1000000000.0f / (maxRPS * (float)spr));

  // Accel — Austin recurrence: T_{n+1} = T_n - 2·T_n/(4n+1)
  unsigned long prevUs = micros();
  for (long n = 1; n <= accelSteps; n++) {
    if (stepNs < cruiseNs) stepNs = cruiseNs;
    unsigned long period  = stepNs / 1000;  // ns → µs
    unsigned long elapsed = micros() - prevUs;
    unsigned long remain  = (elapsed < period) ? (period - elapsed) : 2UL;
    driver.step(remain);
    prevUs = micros();
    stepNs -= 2UL * stepNs / (unsigned long)(4 * n + 1);
  }
  // Cruise
  for (long i = 0; i < cruiseSteps; i++) {
    driver.step(cruisePeriod);
  }
  // Decel — inverse Austin recurrence: T_{n-1} = T_n + 2·T_n/(4n-5)
  stepNs = cruiseNs;
  prevUs = micros();
  for (long n = accelSteps; n >= 1; n--) {
    unsigned long period  = stepNs / 1000;
    unsigned long elapsed = micros() - prevUs;
    unsigned long remain  = (elapsed < period) ? (period - elapsed) : 2UL;
    driver.step(remain);
    prevUs = micros();
    if (n >= 2)
      stepNs += 2UL * stepNs / (unsigned long)(4 * n - 5);
  }
}
```

## Frequency ceiling

| Approach | Accel/decel ceiling |
|---|---|
| sqrt() + micros() | ~4.7 kHz (sqrt exceeds period at 5 kHz) |
| Austin µs (no micros) | stalls early due to integer precision |
| **Austin ns + micros()** | **~33–50 kHz** (limited by 2× 32-bit divide ~25 µs) |

## Previous fix (micros() compensation)

For reference, the intermediate fix that was working before this attempt:

```cpp
void runTrap(float maxRPS, float accelRevSec2, float cruiseSec) {
  int spr = driver.stepsPerRev();
  long accelSteps  = max(1L, (long)(maxRPS * maxRPS * spr / (2.0f * accelRevSec2)));
  long cruiseSteps = (long)(cruiseSec * maxRPS * spr);
  unsigned long cruisePeriod = (unsigned long)(1000000.0f / (maxRPS * spr));

  // Accel
  unsigned long prevUs = micros();
  for (long i = 1; i <= accelSteps; i++) {
    float rps = sqrt(2.0f * accelRevSec2 * (float)i / spr);
    if (rps > maxRPS) rps = maxRPS;
    unsigned long period  = (unsigned long)(1000000.0f / (rps * spr));
    unsigned long elapsed = micros() - prevUs;
    unsigned long remain  = (elapsed < period) ? (period - elapsed) : 2UL;
    driver.step(remain);
    prevUs = micros();
  }
  // Cruise
  for (long i = 0; i < cruiseSteps; i++) {
    driver.step(cruisePeriod);
  }
  // Decel (symmetric reverse of accel)
  prevUs = micros();
  for (long i = accelSteps; i >= 1; i--) {
    float rps = sqrt(2.0f * accelRevSec2 * (float)i / spr);
    if (rps > maxRPS) rps = maxRPS;
    unsigned long period  = (unsigned long)(1000000.0f / (rps * spr));
    unsigned long elapsed = micros() - prevUs;
    unsigned long remain  = (elapsed < period) ? (period - elapsed) : 2UL;
    driver.step(remain);
    prevUs = micros();
  }
}
```
