// H-Bridge Stepper Resonance Test — continuous run, speed controlled by two buttons
// Motor coil A: OUT1/OUT2 | Motor coil B: OUT3/OUT4
// IN1=D49, IN2=D48, IN3=D47, IN4=D46 | ENA=D10, ENB=D9
// Btn UP: D22, Btn DOWN: D23

#include "lib/driver/stepper/l298n.h"

//                     in1  in2  in3  in4  ena  enb  spr  duty
L298N driver(49, 48, 47, 46,  10,   9, 200, (uint8_t)(0.20f * 255));

// ── CONFIGURATION ──────────────────────────────────────────────────────────────
const int   BTN_UP      = 22;
const int   BTN_DOWN    = 23;

const float MIN_RPS     = 0.1f;
const float MAX_RPS     = 160.0f;
const float RPS_STEP    = 0.01f;
const int   DEBOUNCE_MS = 150;
// ──────────────────────────────────────────────────────────────────────────────

float         currentRPS     = 0.2f;
unsigned long lastStepMicros = 0;
unsigned long lastUpMs       = 0;
unsigned long lastDownMs     = 0;

void setup() {
  Serial.begin(115200);

  driver.init();
  driver.setDirection(true);
  driver.enable();
  delay(500);  // let rotor align to phase 0 before stepping

  pinMode(BTN_UP,   INPUT);   // external pull-down: HIGH = pressed
  pinMode(BTN_DOWN, INPUT);

  Serial.println("=== H-BRIDGE STEPPER RESONANCE TEST ===");
  Serial.println("D22=UP  D23=DOWN");
  Serial.print("Start: "); Serial.print(currentRPS, 2); Serial.println(" RPS");
}

void loop() {
  unsigned long nowUs = micros();
  unsigned long nowMs = millis();

  // ── Non-blocking step ──────────────────────────────────────────────────────
  // advance() writes the next phase with no delay; timing is managed here.
  unsigned long stepPeriod = (unsigned long)(1000000.0f / (currentRPS * driver.stepsPerRev()));
  if (nowUs - lastStepMicros >= stepPeriod) {
    driver.advance();
    lastStepMicros = nowUs;
  }

  // ── Button UP ──────────────────────────────────────────────────────────────
  if (digitalRead(BTN_UP) == HIGH && nowMs - lastUpMs > DEBOUNCE_MS) {
    lastUpMs = nowMs;
    currentRPS += RPS_STEP;
    if (currentRPS > MAX_RPS) currentRPS = MAX_RPS;
    Serial.print("RPS: "); Serial.print(currentRPS, 2);
    Serial.print("  | Hz: "); Serial.println(currentRPS * driver.stepsPerRev(), 1);
  }

  // ── Button DOWN ────────────────────────────────────────────────────────────
  if (digitalRead(BTN_DOWN) == HIGH && nowMs - lastDownMs > DEBOUNCE_MS) {
    lastDownMs = nowMs;
    currentRPS -= RPS_STEP;
    if (currentRPS < MIN_RPS) currentRPS = MIN_RPS;
    Serial.print("RPS: "); Serial.print(currentRPS, 2);
    Serial.print("  | Hz: "); Serial.println(currentRPS * driver.stepsPerRev(), 1);
  }
}
