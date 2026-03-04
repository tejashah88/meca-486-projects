// Continuous Run — Z-Axis Motor, speed controlled by two buttons
// Direction: D24, Step: D25
// Btn UP: D22, Btn DOWN: D23
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);

// ── CONFIGURATION ─────────────────────────────────────────────────────────
const int   DIR_PIN    = 24;
const int   STEP_PIN   = 25;
const int   BTN_UP     = 22;
const int   BTN_DOWN   = 23;
const int   SPR        = 200;    // steps/rev — match your driver setting

const float MIN_RPS    = 0.1f;
const float MAX_RPS    = 160.0f;
const float RPS_STEP   = 0.02f;  // RPS change per button press
const int   DEBOUNCE_MS = 150;   // ms between accepted button presses
// ──────────────────────────────────────────────────────────────────────────

float currentRPS = 1.0f;

unsigned long lastStepMicros  = 0;
bool          stepHigh        = false;

unsigned long lastUpMs        = 0;
unsigned long lastDownMs      = 0;
unsigned long lastLCDMs       = 0;

void setup() {
  Serial.begin(115200);

  pinMode(DIR_PIN,   OUTPUT);
  pinMode(STEP_PIN,  OUTPUT);
  pinMode(BTN_UP,   INPUT);   // external pull-down: HIGH = pressed
  pinMode(BTN_DOWN, INPUT);   // external pull-down: HIGH = pressed

  digitalWrite(DIR_PIN,  LOW);
  digitalWrite(STEP_PIN, LOW);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Z-Axis Run");
  lcd.setCursor(0, 1);
  lcd.print("RPS: "); lcd.print(currentRPS, 2);

  Serial.println("=== Z-AXIS CONTINUOUS RUN ===");
  Serial.println("D22=UP  D23=DOWN");
  Serial.print("Start: "); Serial.print(currentRPS, 2); Serial.println(" RPS");
}

void loop() {
  unsigned long nowUs = micros();
  unsigned long nowMs = millis();

  // ── Non-blocking step ─────────────────────────────────────────────
  unsigned long halfPeriod = (unsigned long)(500000.0f / (currentRPS * SPR));
  if (nowUs - lastStepMicros >= halfPeriod) {
    stepHigh = !stepHigh;
    digitalWrite(STEP_PIN, stepHigh ? HIGH : LOW);
    lastStepMicros = nowUs;
  }

  // ── Button UP ─────────────────────────────────────────────────────
  if (digitalRead(BTN_UP) == HIGH && nowMs - lastUpMs > DEBOUNCE_MS) {
    lastUpMs = nowMs;
    currentRPS += RPS_STEP;
    if (currentRPS > MAX_RPS) currentRPS = MAX_RPS;
    Serial.print("RPS: "); Serial.println(currentRPS, 2);
  }

  // ── Button DOWN ───────────────────────────────────────────────────
  if (digitalRead(BTN_DOWN) == HIGH && nowMs - lastDownMs > DEBOUNCE_MS) {
    lastDownMs = nowMs;
    currentRPS -= RPS_STEP;
    if (currentRPS < MIN_RPS) currentRPS = MIN_RPS;
    Serial.print("RPS: "); Serial.println(currentRPS, 2);
  }

  // ── LCD update @ 10 Hz ────────────────────────────────────────────
  if (nowMs - lastLCDMs >= 100) {
    lastLCDMs = nowMs;
    lcd.setCursor(0, 0);
    lcd.print(currentRPS, 2); lcd.print(" RPS    ");
    lcd.setCursor(0, 1);
    lcd.print((long)(currentRPS * SPR)); lcd.print(" Hz      ");
  }
}
