# Motor-Only Stall & Resonance Test Results

**Motor:** STR3 Stepper  
**Driver:** STR3 Stepper Driver  
**Load:** Rotating mass only (no linear stage)  
**Date:** _______________  

---

## Test Setup

- **Program:** `motor_only_test.ino`
- **No limit switches** — SENSOR_PIN_1/2 pulled HIGH internally, limit checking disabled
- **Short press** → Resonance test (constant velocity sweep)
- **Long press >1s** → Stall test (increasing acceleration sweep)
- Motor runs forward then backward each level to keep net rotation near zero

---

## Resonance Tests

### Full Step — 200 steps/rev

**Date:** _______________

| Level | Speed (RPS) | Step Rate (Hz) | Vibration (0–3 / X) |
|-------|-------------|----------------|----------------------|
| 1     | 0.10        | 20             |                      |
| 2     | 0.20        | 40             |                      |
| 3     | 0.30        | 60             |                      |
| 4     | 0.40        | 80             |                      |
| 5     | 0.50        | 100            |                      |
| 6     | 0.60        | 120            |                      |
| 7     | 0.70        | 140            |                      |
| 8     | 0.80        | 160            |                      |
| 9     | 0.90        | 180            |                      |
| 10    | 1.00        | 200            |                      |
| 11    | 1.25        | 250            |                      |
| 12    | 1.50        | 300            |                      |
| 13    | 1.75        | 350            |                      |
| 14    | 2.00        | 400            |                      |
| 15    | 2.50        | 500            |                      |
| 16    | 3.00        | 600            |                      |
| 17    | 3.50        | 700            |                      |
| 18    | 4.00        | 800            |                      |
| 19    | 4.50        | 900            |                      |
| 20    | 5.00        | 1000           |                      |
| 21    | 6.00        | 1200           |                      |
| 22    | 7.00        | 1400           |                      |
| 23    | 8.00        | 1600           |                      |
| 24    | 9.00        | 1800           |                      |
| 25    | 10.0        | 2000           |                      |
| 26    | 12.0        | 2400           |                      |
| 27    | 15.0        | 3000           |                      |

**Primary resonance zone:** ___ – ___ RPS (___ – ___ Hz)  
**Natural frequency estimate:** ___ Hz  
**Notes:**

---

### 1/2 Step — 400 steps/rev

**Date:** _______________

| Level | Speed (RPS) | Step Rate (Hz) | Vibration (0–3 / X) |
|-------|-------------|----------------|----------------------|
| 1     | 0.10        | 40             |                      |
| 2     | 0.20        | 80             |                      |
| 3     | 0.30        | 120            |                      |
| 4     | 0.40        | 160            |                      |
| 5     | 0.50        | 200            |                      |
| 6     | 0.60        | 240            |                      |
| 7     | 0.70        | 280            |                      |
| 8     | 0.80        | 320            |                      |
| 9     | 0.90        | 360            |                      |
| 10    | 1.00        | 400            |                      |
| 11    | 1.25        | 500            |                      |
| 12    | 1.50        | 600            |                      |
| 13    | 1.75        | 700            |                      |
| 14    | 2.00        | 800            |                      |
| 15    | 2.50        | 1000           |                      |
| 16    | 3.00        | 1200           |                      |
| 17    | 3.50        | 1400           |                      |
| 18    | 4.00        | 1600           |                      |
| 19    | 4.50        | 1800           |                      |
| 20    | 5.00        | 2000           |                      |

**Primary resonance zone:** ___ – ___ RPS  
**Notes:**

---

### 1/8 Step — 1600 steps/rev

**Date:** _______________

| Level | Speed (RPS) | Step Rate (Hz) | Vibration (0–3 / X) |
|-------|-------------|----------------|----------------------|
| 1     | 0.10        | 160            |                      |
| 2     | 0.20        | 320            |                      |
| 3     | 0.30        | 480            |                      |
| 4     | 0.40        | 640            |                      |
| 5     | 0.50        | 800            |                      |
| 6     | 0.60        | 960            |                      |
| 7     | 0.70        | 1120           |                      |
| 8     | 0.80        | 1280           |                      |
| 9     | 0.90        | 1440           |                      |
| 10    | 1.00        | 1600           |                      |
| 11    | 1.25        | 2000           |                      |
| 12    | 1.50        | 2400           |                      |
| 13    | 1.75        | 2800           |                      |
| 14    | 2.00        | 3200           |                      |
| 15    | 2.50        | 4000           |                      |
| 16    | 3.00        | 4800           |                      |
| 17    | 3.50        | 5600           |                      |
| 18    | 4.00        | 6400           |                      |
| 19    | 4.50        | 7200           |                      |
| 20    | 5.00        | 8000           |                      |

**Primary resonance zone:** ___ – ___ RPS  
**Notes:**

---

### Additional Microstep Settings

> Duplicate the table above for each additional setting tested (400, 800, 1000, 3200, etc.)

---

## Stall Tests

### Full Step — 200 steps/rev (Cruise: 10 RPS)

**Date:** _______________

| Level | Ramp (rev) | Accel (rev/s²) | Result (Pass / Stall) |
|-------|-----------|----------------|-----------------------|
| 1     | 3.00      | 16.7           |                       |
| 2     | 2.00      | 25.0           |                       |
| 3     | 1.50      | 33.3           |                       |
| 4     | 1.00      | 50.0           |                       |
| 5     | 0.75      | 66.7           |                       |
| 6     | 0.50      | 100.0          |                       |
| 7     | 0.35      | 142.9          |                       |
| 8     | 0.25      | 200.0          |                       |
| 9     | 0.15      | 333.3          |                       |
| 10    | 0.10      | 500.0          |                       |
| 11    | 0.05      | 1000.0         |                       |

**Last passing level:** Lv ___, Ramp = ___ rev, Accel = ___ rev/s²  
**First stalling level:** Lv ___, Ramp = ___ rev, Accel = ___ rev/s²  
**Safe working acceleration (75%):** ___ rev/s²  
**Minimum safe ramp @ 10 RPS:** ___ rev  

---

### 1/8 Step — 1600 steps/rev (Cruise: 10 RPS)

**Date:** _______________

| Level | Ramp (rev) | Accel (rev/s²) | Result (Pass / Stall) |
|-------|-----------|----------------|-----------------------|
| 1     | 3.00      | 16.7           |                       |
| 2     | 2.00      | 25.0           |                       |
| 3     | 1.50      | 33.3           |                       |
| 4     | 1.00      | 50.0           |                       |
| 5     | 0.75      | 66.7           |                       |
| 6     | 0.50      | 100.0          |                       |
| 7     | 0.35      | 142.9          |                       |
| 8     | 0.25      | 200.0          |                       |
| 9     | 0.15      | 333.3          |                       |
| 10    | 0.10      | 500.0          |                       |
| 11    | 0.05      | 1000.0         |                       |

**Last passing level:** Lv ___, Accel = ___ rev/s²  
**Safe working acceleration (75%):** ___ rev/s²  

---

## Comparison: Motor-Only vs KR33 Stage

| Test                      | Motor Only | KR33 Stage | Difference | Notes                        |
|---------------------------|-----------|------------|------------|------------------------------|
| Primary resonance (RPS)   |           | 0.90–1.25  |            |                              |
| Primary resonance (Hz)    |           | 180–250    |            |                              |
| Stall accel @ 10 RPS      |           | 454–500 rev/s² |        |                              |
| Safe accel limit (75%)    |           | 341 rev/s² |            |                              |
| Max unramped speed        |           | <10 RPS    |            |                              |

> The difference between motor-only and stage results shows the effect of the KR33 carriage inertia and lead screw load on motor performance.

---
