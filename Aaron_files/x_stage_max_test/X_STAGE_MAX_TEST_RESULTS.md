# X-Axis Linear Stage — Max Speed & Max Acceleration Test Results

**Stage:** THK KR33  
**Motor/Driver:** STR3  
**Steps/rev:** 200 | **mm/rev:** 6.0  
**Program:** `x_stage_max_test.ino`  
**Method:** SHORT press = Max Speed | LONG press (≥1s) = Max Acceleration

---

## Max Speed Test

**Profile:** 2 rev accel + 6 rev cruise + 2 rev decel (10 rev total), then return to home.

| Level | Cruise RPS | Result (OK / Stall) |
| ----- | ---------- | ------------------- |
| 1     | 8          |                    |
| 2     | 10         |                    |
| 3     | 12         |                    |
| 4     | 14         |                    |
| 5     | 16         |                    |
| 6     | 18         |                    |
| 7     | 20         |                    |
| 8     | 22         |                    |
| 9     | 24         |                    |

**Last passing speed:** _____ RPS  
**First stalling speed:** _____ RPS  
**Max safe speed (e.g. 90%):** _____ RPS  

---

## Max Acceleration Test

**Profile:** Fixed cruise 10 RPS, 5 rev cruise; ramp distance decreased each level → accel = v²/(2·ramp) rev/s².

| Level | Ramp (rev) | Accel (rev/s²) | Result (OK / Stall) |
| ----- | ---------- | ----------------- | ------------------- |
| 1     | 3.00       | 16.7             |                     |
| 2     | 2.00       | 25.0             |                     |
| 3     | 1.50       | 33.3             |                     |
| 4     | 1.00       | 50.0             |                     |
| 5     | 0.75       | 66.7             |                     |
| 6     | 0.50       | 100.0            |                     |
| 7     | 0.35       | 142.9            |                     |
| 8     | 0.25       | 200.0            |                     |
| 9     | 0.15       | 333.3            |                     |
| 10    | 0.10       | 500.0            |                     |
| 11    | 0.05       | 1000.0           |                     |

**Last passing acceleration:** _____ rev/s²  
**First stalling acceleration:** _____ rev/s²  
**Safe working acceleration (e.g. 75%):** _____ rev/s²  

---

## Summary

| Quantity        | Value        |
| ---------------- | ------------ |
| Max speed (RPS)  |              |
| Max accel (rev/s²) |           |
| Safe speed       |              |
| Safe accel       |              |
