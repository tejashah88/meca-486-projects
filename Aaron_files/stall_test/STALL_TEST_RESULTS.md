# STR3 Stepper Motor — Stall Test Results

**Date:** February 24, 2026  
**Operator:** Aaron  
**Motor:** STR3 Stepper  
**Driver:** STR3 Stepper Driver  
**Platform:** THK KR33 Linear Motion Platform  
**Steps/Rev:** 200  
**Load:** Unloaded (carriage only)

---

## Test Setup

- **Program:** `stall_test.ino`
- **Cruise Speed:** 10 RPS
- **Cruise Distance:** 5 rev
- **Method:** Fixed cruise speed, decreasing accel ramp distance → increasing acceleration.
Each level runs a forward move then physically re-homes via the limit sensor (corrects position even after a stall).
- **Acceleration formula:** `a = v² / (2·d)` where v = cruise speed (rev/s), d = ramp distance (rev)

---

## Coarse Sweep


| Level | Ramp (rev) | Accel (rev/s²) | Result      |
| ----- | ---------- | -------------- | ----------- |
| 1     | 3.00       | 16.7           | ✅ Pass      |
| 2     | 2.00       | 25.0           | ✅ Pass      |
| 3     | 1.50       | 33.3           | ✅ Pass      |
| 4     | 1.00       | 50.0           | ✅ Pass      |
| 5     | 0.75       | 66.7           | ✅ Pass      |
| 6     | 0.50       | 100.0          | ✅ Pass      |
| 7     | 0.35       | 142.9          | ✅ Pass      |
| 8     | 0.25       | 200.0          | ✅ Pass      |
| 9     | 0.15       | 333.3          | ✅ Pass      |
| 10    | 0.10       | 500.0          | ❌ Stall     |
| 11    | 0.05       | 1000.0         | — (skipped) |


---

## Fine Sweep (0.10 – 0.15 rev)


| Level | Ramp (rev) | Accel (rev/s²) | Result  |
| ----- | ---------- | -------------- | ------- |
| 1     | 0.15       | 333.3          | ✅ Pass  |
| 2     | 0.14       | 357.1          | ✅ Pass  |
| 3     | 0.13       | 384.6          | ✅ Pass  |
| 4     | 0.12       | 416.7          | ✅ Pass  |
| 5     | 0.11       | 454.5          | ✅ Pass  |
| 6     | 0.10       | 500.0          | ❌ Stall |


---

## Results Summary


| Parameter                      | Value                            |
| ------------------------------ | -------------------------------- |
| Last passing acceleration      | **454.5 rev/s²** (0.11 rev ramp) |
| First stalling acceleration    | **500.0 rev/s²** (0.10 rev ramp) |
| Stall threshold (estimated)    | **~477 rev/s²**                  |
| Safety factor applied          | 75%                              |
| **Safe working acceleration**  | **≤ 341 rev/s²**                 |
| **Minimum safe ramp @ 10 RPS** | **≥ 0.15 rev**                   |


---

## Design Rule

For any cruise speed, the minimum ramp distance is:

$$\text{ramp (rev)} = \frac{v^2}{2 \times 341}$$

where v is cruise speed in rev/s (RPS).


| Cruise Speed (RPS) | Min Safe Ramp (rev) |
| ------------------ | ------------------- |
| 5                  | 0.04                |
| 10                 | 0.15                |
| 15                 | 0.33                |
| 20                 | 0.59                |
| 30                 | 1.32                |
| 40                 | 2.35                |


> **Note:** This characterization was done unloaded. Add additional margin when running under load.

