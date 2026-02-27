# MECA 486 — Motion Control Lab Report

**Author:** Aaron  
**Date:** February 2026  

---

## Table of Contents

1. [H-Bridge (Z-Axis Motion)](#1-h-bridge-z-axis-motion)
2. [Stepper Drive Implementation (Z-Axis Motion)](#2-stepper-drive-implementation-z-axis-motion)
3. [Stepper Driven Stage Implementation (X-Axis Motion)](#3-stepper-driven-stage-implementation-x-axis-motion)

---

## 1. H-Bridge (Z-Axis Motion)

### 1.1 Wiring Diagram

> *Insert wiring diagram image here.*


| Signal | Pin |
| ------ | --- |
|        |     |
|        |     |


---

### 1.2 Trapezoidal Move with Defined Distance

> *Describe the motion profile used. Include code snippet or parameters.*

**Profile parameters:**


| Parameter             | Value |
| --------------------- | ----- |
| Acceleration distance |       |
| Cruise distance       |       |
| Deceleration distance |       |
| Cruise speed          |       |
| Total distance        |       |


---

### 1.3 Reverse Direction

> *Describe how direction reversal is implemented (direction pin, H-bridge logic, etc.).*

---

### 1.4 Start Move with Button Push

> *Describe button wiring and debounce logic.*

---

### 1.5 Current Control (PWM)

> *PWM is used for current control rather than V = iR because the motor winding inductance
> smooths the current, making average current proportional to duty cycle regardless of winding resistance.*


| Parameter            | Value |
| -------------------- | ----- |
| PWM frequency (Hz)   |       |
| Duty cycle (%)       |       |
| Target current (A)   |       |
| Measured current (A) |       |


---

### 1.6 Resonance Region Testing & Results

> *Stepper motors exhibit resonance at certain step frequencies where mechanical oscillations
> build up and cause vibration or stall.*


| Step Frequency (Hz) | Observed Behavior |
| ------------------- | ----------------- |
|                     |                   |
|                     |                   |


**Resonance frequency identified:** _____ Hz  
**Mitigation strategy:** *e.g., ramp through quickly, microstepping, damping*

---

### 1.7 Discovered Limitations

- 

- 

- 

---

## 2. Stepper Drive Implementation (Z-Axis Motion)

### 2.1 Wiring Diagram

> *Insert wiring diagram image here.*


| Signal | Pin |
| ------ | --- |
|        |     |
|        |     |


---

### 2.2 Drive Setup

> *List all driver settings as configured.*


| Setting               | Value             |
| --------------------- | ----------------- |
| Drive model           |                   |
| Microstep setting     |                   |
| Current limit (A)     |                   |
| Bus voltage (V)       |                   |
| Winding configuration | Series / Parallel |


---

### 2.3 Trapezoidal Move with Defined Distance

**Profile parameters:**


| Parameter             | Value |
| --------------------- | ----- |
| Acceleration distance |       |
| Cruise distance       |       |
| Deceleration distance |       |
| Cruise speed          |       |
| Total distance        |       |


---

### 2.4 Reverse Direction

> *Describe direction reversal implementation.*

---

### 2.5 Start Move with Button Push

> *Describe button wiring and debounce logic.*

---

### 2.6 Resonance Region Testing & Results


| Step Frequency (Hz) | Observed Behavior |
| ------------------- | ----------------- |
|                     |                   |
|                     |                   |


**Resonance frequency identified:** _____ Hz

---

### 2.7 Effect of Microstepping Size


| Microstep Setting | Steps/Rev | Observed Effect on Smoothness / Noise / Speed |
| ----------------- | --------- | --------------------------------------------- |
| Full step (1)     | 200       |                                               |
| Half step (2)     | 400       |                                               |
| 1/4               | 800       |                                               |
| 1/8               | 1600      |                                               |
| 1/16              | 3200      |                                               |


---

### 2.8 Effect of Varying Bus Voltage


| Bus Voltage (V) | Max Speed Achieved | Notes |
| --------------- | ------------------ | ----- |
|                 |                    |       |
|                 |                    |       |


---

### 2.9 Effect of Varying Current


| Current Setting (A) | Torque / Stall Behavior | Temperature | Notes |
| ------------------- | ----------------------- | ----------- | ----- |
|                     |                         |             |       |
|                     |                         |             |       |


---

### 2.10 Effect of Winding Configuration (Series vs. Parallel)


| Configuration | Inductance | Max Speed | Torque at Low Speed | Notes |
| ------------- | ---------- | --------- | ------------------- | ----- |
| Series        | Higher     | Lower     | Higher              |       |
| Parallel      | Lower      | Higher    | Lower               |       |


---

## 3. Stepper Driven Stage Implementation (X-Axis Motion)

**Platform:** THK KR33 Linear Motion Stage  
**Motor:** STR3 Stepper  
**Driver:** STR3 Stepper Driver  
**Steps/Rev:** 200  

---

### 3.1 Wiring Diagram

> *Insert wiring diagram image here.*


| Signal                    | Pin |
| ------------------------- | --- |
| DIR                       | D9  |
| STEP                      | D10 |
| BUTTON                    | D22 |
| End limit (SENSOR_PIN_1)  | D2  |
| Home limit (SENSOR_PIN_2) | D3  |
| LCD RS                    | D7  |
| LCD EN                    | D8  |
| LCD D4                    | D4  |
| LCD D5                    | D5  |
| LCD D6                    | D6  |
| LCD D7                    | D11 |


---

### 3.2 Drive Setup


| Setting               | Value             |
| --------------------- | ----------------- |
| Drive model           | STR3              |
| Microstep setting     |                   |
| Current limit (A)     |                   |
| Bus voltage (V)       |                   |
| Winding configuration | Series / Parallel |


---

### 3.3 X-Axis Motion with Defined Distances

**Profile parameters:**


| Move | Accel (rev) | Cruise (rev) | Decel (rev) | Cruise Speed (RPS) |
| ---- | ----------- | ------------ | ----------- | ------------------ |
| 1    | 2.0         | 40.0         | 2.0         | 15                 |
| 2    | 2.0         | 10.0         | 2.0         | 15                 |
| 3    | 2.0         | 50.0         | 2.0         | 10                 |
| 4    | 10.0        | 0            | 20.0        | 10                 |


---

### 3.4 Limits Operational


| Limit               | Pin | Trigger State | Behavior                   |
| ------------------- | --- | ------------- | -------------------------- |
| Home (SENSOR_PIN_2) | D3  | LOW           | Stops toward-home movement |
| End (SENSOR_PIN_1)  | D2  | LOW           | Stops toward-end movement  |


Direction-aware limit checking — only blocks movement toward the triggered switch,
allowing the motor to always reverse away from a limit.

---

### 3.5 Homing Routine

1. On startup, `calibrateAxis(0.5 RPS)` is called
2. Motor creeps toward home at 0.5 RPS until SENSOR_PIN_2 triggers → `motorPosition = 0`
3. Motor creeps toward end at 0.5 RPS until SENSOR_PIN_1 triggers → `endPosition` and `axisLength` recorded
4. `moveToHome(15 RPS)` returns carriage to home with a trapezoidal profile

---

### 3.6 Resonance Region Testing & Results

**Test date:** February 24, 2026  
**Microstep setting:** Full step (200 steps/rev)  
**Method:** Constant velocity runs (`rotate()`), no accel/decel ramp. Vibration rated 0–3 (0=none, 3=strongest). X=stall.

| Level | Speed (RPS) | Step Rate (Hz) | Vibration |
|-------|-------------|----------------|-----------|
| 1     | 0.10        | 20             | 1         |
| 2     | 0.20        | 40             | 1         |
| 3     | 0.30        | 60             | 1         |
| 4     | 0.40        | 80             | 1         |
| 5     | 0.50        | 100            | 2         |
| 6     | 0.60        | 120            | 2         |
| 7     | 0.70        | 140            | 1         |
| 8     | 0.80        | 160            | 2         |
| 9     | 0.90        | 180            | 3         |
| 10    | 1.00        | 200            | 3         |
| 11    | 1.25        | 250            | 3         |
| 12    | 1.50        | 300            | 1         |
| 13    | 1.75        | 350            | 0         |
| 14    | 2.00        | 400            | 1         |
| 15    | 2.50        | 500            | 2         |
| 16    | 3.00        | 600            | 1         |
| 17    | 3.50        | 700            | 0         |
| 18    | 4.00        | 800            | —         |
| 19    | 4.50        | 900            | 0         |
| 20    | 5.00        | 1000           | 0         |
| 21    | 6.00        | 1200           | 0         |
| 22    | 7.00        | 1400           | 0         |
| 23    | 8.00        | 1600           | 0         |
| 24    | 9.00        | 1800           | 0         |
| 25    | 10.0        | 2000           | X (stall) |

**Resonance zones identified (full step, 200 steps/rev):**

| Zone      | Speed Range (RPS) | Step Rate Range (Hz) | Peak Vibration | Notes                                              |
|-----------|-------------------|----------------------|----------------|----------------------------------------------------|
| **Primary**   | **0.90 – 1.25** | **180 – 250**    | **3**          | **Sharp, dominant peak — significantly worse than all other levels. Avoid entirely.** |
| Secondary | 0.50 – 0.60       | 100 – 120            | 2              | Mild compared to primary                           |
| Tertiary  | 2.50              | 500                  | 2              | Mild compared to primary                           |

**Natural frequency of system:** ~215 Hz step rate (~1.1 RPS at full step)

> The primary resonance at levels 9–11 (0.90–1.25 RPS) was dramatically worse than all other speeds, indicating a sharp mechanical resonance rather than a gradual vibration trend. This is the natural frequency of the KR33 carriage + motor rotor system.

> **Note on stall at 10 RPS:** `rotate()` starts at full speed instantly with no ramp. This stall is likely torque-curve limited rather than resonance-induced — the trapezoidal profiler with ramps ran successfully at 10 RPS during stall testing.

---

### 3.7 Effect of Microstepping Size


| Microstep Setting | Steps/Rev | Smoothness | Noise | Max Speed | Notes |
| ----------------- | --------- | ---------- | ----- | --------- | ----- |
|                   |           |            |       |           |       |
|                   |           |            |       |           |       |


---

### 3.8 Effect of Varying Bus Voltage


| Bus Voltage (V) | Max Speed Achieved (RPS) | Notes |
| --------------- | ------------------------ | ----- |
|                 |                          |       |
|                 |                          |       |


---

### 3.9 Effect of Varying Current


| Current Setting (A) | Stall Acceleration (rev/s²) | Notes |
| ------------------- | --------------------------- | ----- |
|                     |                             |       |
|                     |                             |       |


---

### 3.10 Effect of Winding Configuration (Series vs. Parallel)


| Configuration | Max Speed (RPS) | Stall Accel (rev/s²) | Notes |
| ------------- | --------------- | -------------------- | ----- |
| Series        |                 |                      |       |
| Parallel      |                 |                      |       |


---

### 3.11 Accuracy of Moves

> *Measure actual carriage displacement vs. commanded displacement.*

**Measurement method:** *e.g., calipers, dial indicator*  
**Lead screw pitch:** _____ mm/rev  
**Theoretical resolution:** _____ mm/step  


| Commanded Distance (mm) | Measured Distance (mm) | Error (mm) | Error (%) |
| ----------------------- | ---------------------- | ---------- | --------- |
|                         |                        |            |           |
|                         |                        |            |           |
|                         |                        |            |           |


---

### 3.12 Repeatability of Moves

> *Command the same move multiple times from the same starting position and measure variation.*


| Trial | Measured Position (mm) |
| ----- | ---------------------- |
| 1     |                        |
| 2     |                        |
| 3     |                        |
| 4     |                        |
| 5     |                        |


**Mean:** _____ mm  
**Std. Dev.:** _____ mm  
**Repeatability (±):** _____ mm  

---

### 3.13 Actual Resolution vs. Theoretical


|                                  | Value |
| -------------------------------- | ----- |
| Steps/Rev                        | 200   |
| Lead screw pitch (mm/rev)        |       |
| Theoretical resolution (mm/step) |       |
| Measured minimum move (mm)       |       |
| Backlash observed (mm)           |       |


---

### 3.14 Stall Testing Results

> See [`stall_test/STALL_TEST_RESULTS.md`](stall_test/STALL_TEST_RESULTS.md) for full data.

| Condition                       | Stall Threshold (rev/s²) | Safe Limit @ 75% (rev/s²) |
| ------------------------------- | ------------------------ | ------------------------- |
| Unloaded, 10 RPS, KR33 carriage | 454.5 – 500              | 341                       |
| Loaded                          |                          |                           |

**Design rule (unloaded):** minimum ramp (rev) = v² / 682, where v is cruise speed in RPS.

---

### 3.15 `profileMove` Discovered Limitations

The following limitations were identified through stall testing and resonance testing of the trapezoidal motion profiler:

**1. Acceleration limit (stall testing)**
- Stall occurs between 454.5 and 500 rev/s² at 10 RPS (unloaded, full step, KR33 carriage)
- Safe working acceleration: ≤ 341 rev/s² (75% margin)
- Minimum safe ramp distance: `ramp (rev) = v² / 682`
- All tested `profileMove` calls use ramps of ≥ 2 rev at ≤ 15 RPS (~56 rev/s²) — well within the safe zone

**2. Speed limit (resonance & torque-speed curve)**
- At full step (200 steps/rev), the motor stalls when `rotate()` attempts 10 RPS with no ramp
- `profileMove` successfully ran at 10 RPS with a ramp, indicating 10 RPS is near the practical ceiling at this bus voltage and current
- Operating above ~8–9 RPS leaves little torque margin for disturbances or loads

**3. Resonance zone (resonance testing)**
- Sharp mechanical resonance at 0.90–1.25 RPS (180–250 Hz step rate) at full step
- Vibration at resonance was dramatically worse than all other speeds
- `profileMove` ramps through this zone quickly rather than dwelling in it — this is the correct behavior
- Sustained constant-velocity operation in this range must be avoided

**4. No position feedback**
- `profileMove` is open-loop — missed steps from resonance or overload silently corrupt `motorPosition`
- `homeAxis()` must be called after any suspected stall to resync position from the physical limit switch

**5. Arduino pulse rate ceiling**
- Step pulses are generated with `delayMicroseconds()`, which is unreliable below ~5 µs half-period
- Maximum reliable step rate: ~100 kHz → maximum RPS = 100,000 / STEPS_PER_REV
- At full step: 500 RPS limit (not a practical concern)
- At 1/8 step (1600 steps/rev): 62.5 RPS limit
- At 1/128 step (25600 steps/rev): 3.9 RPS limit — becomes a real constraint at high microstepping

---

