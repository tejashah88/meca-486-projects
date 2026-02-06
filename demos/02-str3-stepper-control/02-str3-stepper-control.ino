// Hardware pin setup

const int STEP_PIN   = 10;
const int DIR_PIN    = 9;

// Stepper motor control

const int STEPS_PER_REV = 3200;
const int RPM = 100;
unsigned long STEP_PERIOD_US = 60000000UL / ((unsigned long) RPM * STEPS_PER_REV);

// Split into HIGH and LOW times
const unsigned long STEP_PULSE_US = 250;

// Idle time between pulses
const unsigned long STEP_IDLE_US = (STEP_PERIOD_US > STEP_PULSE_US) ? (STEP_PERIOD_US - STEP_PULSE_US) : 0;


void setup() {
  // Setup up configuration pins
  pinMode(STEP_PIN, OUTPUT); // 0 = stepping, 1 = not stepping
  pinMode(DIR_PIN, OUTPUT);  // 0 = CW, 1 = CCW

  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, HIGH);
}


void rotate_n_steps(int steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, LOW);      // close STEP to execute 1 step
    delayMicroseconds(STEP_PULSE_US);

    digitalWrite(STEP_PIN, HIGH);     // open STEP
    delayMicroseconds(STEP_IDLE_US);
  }
}


void loop() {
  digitalWrite(DIR_PIN, LOW); // LOW = 0 = CW
  rotate_n_steps(STEPS_PER_REV);
  delay(1000);

  digitalWrite(DIR_PIN, HIGH); // HIGH = 1 = CCW
  rotate_n_steps(STEPS_PER_REV);
  delay(1000);
}
