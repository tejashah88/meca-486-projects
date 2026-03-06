// Continuous Run — X-Axis Stage, speed controlled by two buttons
// Direction: D51, Step: D53
// Limits: D2 (End), D3 (Home) — auto-reverses on trigger
// Btn UP: D22, Btn DOWN: D23
// LCD: RS=7, EN=8, D4=4, D5=5, D6=6, D7=11

#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 4, 5, 6, 11);

// ── CONFIGURATION ─────────────────────────────────────────────────────────
const int   DIR_PIN     = 51;
const int   STEP_PIN    = 53;
const int   LIMIT_END   = 2;    // INPUT_PULLUP — LOW = triggered
const int   LIMIT_HOME  = 3;    // INPUT_PULLUP — LOW = triggered
const int   BTN_UP      = 22;
const int   BTN_DOWN    = 23;
const int   SPR         = 200;   // steps/rev — match driver setting

const float MIN_RPS     = 0.1f;
const float MAX_RPS     = 20.0f;
const float RPS_STEP    = 0.05f;  // RPS change per button press
const int   DEBOUNCE_MS = 150;
// ──────────────────────────────────────────────────────────────────────────

float currentRPS = 0.5f;
bool  dirForward = true;  // true = toward end (LOW on DIR pin)

unsigned long lastStepMicros = 0;
bool          stepHigh       = false;

unsigned long lastUpMs   = 0;
unsigned long lastDownMs = 0;
unsigned long lastLCDMs  = 0;

void setDir(bool forward) {
  dirForward = forward;
  digitalWrite(DIR_PIN, forward ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);

  pinMode(DIR_PIN,    OUTPUT);
  pinMode(STEP_PIN,   OUTPUT);
  pinMode(LIMIT_END,  INPUT_PULLUP);
  pinMode(LIMIT_HOME, INPUT_PULLUP);
  pinMode(BTN_UP,   INPUT);   // external pull-down: HIGH = pressed
  pinMode(BTN_DOWN, INPUT);   // external pull-down: HIGH = pressed

  setDir(true);
  digitalWrite(STEP_PIN, LOW);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("X-Axis Run");
  lcd.setCursor(0, 1);
  lcd.print(currentRPS, 2); lcd.print(" RPS");

  Serial.println("=== X-AXIS CONTINUOUS RUN ===");
  Serial.println("D22=UP (+0.05 RPS)  D23=DOWN (-0.05 RPS)");
  Serial.print("Start: "); Serial.print(currentRPS, 2); Serial.println(" RPS");
}

void loop() {
  unsigned long nowUs = micros();
  unsigned long nowMs = millis();

  // ── Limit check — flip direction instantly ────────────────────────────
  if (digitalRead(LIMIT_END)  == LOW && dirForward)  {
    setDir(false);
    Serial.println("  [END — reversing]");
  }
  if (digitalRead(LIMIT_HOME) == LOW && !dirForward) {
    setDir(true);
    Serial.println("  [HOME — reversing]");
  }

  // ── Non-blocking step ─────────────────────────────────────────────────
  unsigned long halfPeriod = (unsigned long)(500000.0f / (currentRPS * SPR));
  if (nowUs - lastStepMicros >= halfPeriod) {
    stepHigh = !stepHigh;
    digitalWrite(STEP_PIN, stepHigh ? HIGH : LOW);
    lastStepMicros = nowUs;
  }

  // ── Button UP ─────────────────────────────────────────────────────────
  if (digitalRead(BTN_UP) == HIGH && nowMs - lastUpMs > DEBOUNCE_MS) {
    lastUpMs = nowMs;
    currentRPS += RPS_STEP;
    if (currentRPS > MAX_RPS) currentRPS = MAX_RPS;
    Serial.print("RPS: "); Serial.print(currentRPS, 2);
    Serial.print("  Hz: "); Serial.println((long)(currentRPS * SPR));
  }

  // ── Button DOWN ───────────────────────────────────────────────────────
  if (digitalRead(BTN_DOWN) == HIGH && nowMs - lastDownMs > DEBOUNCE_MS) {
    lastDownMs = nowMs;
    currentRPS -= RPS_STEP;
    if (currentRPS < MIN_RPS) currentRPS = MIN_RPS;
    Serial.print("RPS: "); Serial.print(currentRPS, 2);
    Serial.print("  Hz: "); Serial.println((long)(currentRPS * SPR));
  }

  // ── LCD update @ 10 Hz ────────────────────────────────────────────────
  if (nowMs - lastLCDMs >= 100) {
    lastLCDMs = nowMs;
    lcd.setCursor(0, 0);
    lcd.print(currentRPS, 2); lcd.print(" RPS    ");
    lcd.setCursor(0, 1);
    lcd.print((long)(currentRPS * SPR)); lcd.print(" Hz      ");
  }
}
