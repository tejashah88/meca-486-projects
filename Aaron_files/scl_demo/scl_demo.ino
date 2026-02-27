// scl_demo.ino
// Demonstrates the SCLMotor library with the Applied Motion ST10-S driver.
//
// Hardware (Arduino Mega):
//   ST10-S RS-232 TX → MAX232 → Arduino RX1 (pin 19)
//   ST10-S RS-232 RX → MAX232 → Arduino TX1 (pin 18)
//   Button    → pin 22 (INPUT_PULLUP, active LOW)
//
// Driver setup (ST Configurator):
//   - Control mode : SCL (Host mode)
//   - Baud rate    : 9600 (factory default)
//   - Steps/rev    : set to match your microstep setting (e.g. 20000)
//
// Demo sequence (triggered by button press):
//   1. Home / zero position
//   2. Move +5 revolutions forward (relative)
//   3. Move to absolute position 0  (back to home)
//   4. Jog forward for 2 seconds, then stop
//   Each step prints status to Serial Monitor (115200 baud).

#include "SCLMotor.h"

// ── Pin assignments ──────────────────────────────────────────────────────────
static const int BUTTON_PIN = 22;

// ── Motion parameters ────────────────────────────────────────────────────────
static const float ACCEL_RPS2  = 20.0f;  // rev/s²
static const float DECEL_RPS2  = 20.0f;  // rev/s²
static const float CRUISE_RPS  =  5.0f;  // rev/s
static const int   STEPS_PER_REV = 20000; // match driver microstep setting

// ── SCL driver config ────────────────────────────────────────────────────────
SCLConfig axis;

// ── Helpers ──────────────────────────────────────────────────────────────────

// Block until button is pressed then released (debounced).
static void waitForButton() {
    Serial.println("  [waiting for button]");
    // Wait for button to be released first (in case it's held)
    while (digitalRead(BUTTON_PIN) == LOW) delay(10);
    // Wait for press
    while (digitalRead(BUTTON_PIN) == HIGH) delay(10);
    // Debounce
    delay(50);
    // Wait for release
    while (digitalRead(BUTTON_PIN) == LOW) delay(10);
    delay(50);
}

static void printPosition() {
    long pos = sclGetPosition(&axis);
    Serial.print("  Position: ");
    Serial.print(pos);
    Serial.println(" steps");
}

// ── Setup ────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("=== SCL Motor Demo ===");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Configure the SCL axis
    axis.port     = &Serial1;
    axis.baudRate = 9600;

    // Start communication and configure driver defaults
    sclBegin(&axis);
    Serial.println("Serial1 open, PR4 + IFD sent.");

    // Clear any latched alarms from previous run
    if (sclHasAlarm(&axis)) {
        Serial.println("Alarm detected — clearing...");
        sclClearAlarm(&axis);
        delay(100);
    }

    // Energise motor
    sclEnable(&axis);
    Serial.println("Motor enabled.");

    // Set motion parameters
    sclSetAccel(&axis, ACCEL_RPS2);
    sclSetDecel(&axis, DECEL_RPS2);
    sclSetVelocity(&axis, CRUISE_RPS);

    // Define current position as zero
    sclSetPosition(&axis, 0);

    Serial.println("Ready. Press button to run demo sequence.");
}

// ── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
    // ── Step 0: wait for button ──────────────────────────────────────────────
    waitForButton();
    Serial.println("\n--- Demo Start ---");

    // ── Step 1: move forward 5 revolutions ──────────────────────────────────
    long forwardSteps = (long)STEPS_PER_REV * 5;
    Serial.print("Step 1: Move forward ");
    Serial.print(forwardSteps);
    Serial.println(" steps (+5 rev)...");

    sclMoveRelative(&axis, forwardSteps);
    bool done = sclWaitForMove(&axis, 10000UL); // 10 s timeout
    if (!done) {
        Serial.println("  ERROR: move timed out!");
        sclStop(&axis);
    }
    printPosition();

    // ── Step 2: return to absolute zero ─────────────────────────────────────
    Serial.println("Step 2: Return to position 0 (absolute)...");
    sclMoveAbsolute(&axis, 0);
    done = sclWaitForMove(&axis, 10000UL);
    if (!done) {
        Serial.println("  ERROR: move timed out!");
        sclStop(&axis);
    }
    printPosition();

    // ── Step 3: jog forward for 2 seconds ───────────────────────────────────
    Serial.println("Step 3: Jog forward at 2 rev/s for 2 seconds...");
    sclJogStart(&axis, 2.0f);
    delay(2000);
    sclJogStop(&axis);
    sclWaitForMove(&axis, 3000UL); // wait for decel to finish
    printPosition();

    // ── Step 4: back to zero ─────────────────────────────────────────────────
    Serial.println("Step 4: Return to position 0...");
    sclMoveAbsolute(&axis, 0);
    done = sclWaitForMove(&axis, 10000UL);
    if (!done) {
        Serial.println("  ERROR: move timed out!");
        sclStop(&axis);
    }
    printPosition();

    Serial.println("--- Demo Complete. Press button to repeat. ---");
}
