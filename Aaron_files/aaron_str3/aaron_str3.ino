// Hardware pin setup

const int STEP_PIN   = 10;
const int DIR_PIN    = 9;

// Stepper motor control

const int STEPS_PER_REV = 200;
const unsigned long STEP_PULSE_US = 250;

// Calculate period from RPM
unsigned long rpmToPeriod(float rpm) {
  if (rpm < 0.1) rpm = 0.1; // Prevent division by zero
  return 60000000UL / (rpm * STEPS_PER_REV);
}


void setup() {
  // Setup up configuration pins
  pinMode(STEP_PIN, OUTPUT); // 0 = stepping, 1 = not stepping
  pinMode(DIR_PIN, OUTPUT);  // 0 = CW, 1 = CCW

  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, HIGH);
}


// Trapezoidal move with specified parameters
// maxVelocityRPM: peak velocity in RPM
// totalTimeMs: total time for move in milliseconds
// revolutions: number of revolutions to move (negative = CCW, positive = CW)
void trapezoidalMove(float maxVelocityRPM, float totalTimeMs, float revolutions) {
  // Set direction based on sign of revolutions
  if (revolutions < 0) {
    digitalWrite(DIR_PIN, HIGH); // CCW
  } else {
    digitalWrite(DIR_PIN, LOW);  // CW
  }
  
  // Use absolute value for calculations
  float absRevolutions = abs(revolutions);
  int totalSteps = (int)(absRevolutions * STEPS_PER_REV);
  float X = totalSteps; // Total distance in steps
  
  // Convert max velocity to steps/ms
  float V = (maxVelocityRPM * STEPS_PER_REV) / 60000.0; // steps/ms
  
  // Calculate acceleration time using: t_a = t_m - X/V
  float accelTimeMs = totalTimeMs - (X / V);
  
  // Check if trapezoidal profile is achievable
  if (accelTimeMs <= 0) {
    // Cannot reach max velocity - use triangular profile: V = 2X/t_m
    accelTimeMs = totalTimeMs / 2.0;
    V = (2.0 * X) / totalTimeMs;
  }
  
  float decelTimeMs = accelTimeMs; // Symmetric profile
  float cruiseTimeMs = totalTimeMs - 2.0 * accelTimeMs;
  
  // Calculate step counts for each phase
  int accelSteps = (int)(0.5 * V * accelTimeMs);
  int decelSteps = accelSteps;
  int cruiseSteps = totalSteps - accelSteps - decelSteps;
  
  // Calculate acceleration (a = V/t_a) in steps/ms^2
  float acceleration = V / accelTimeMs;
  
  // Execute the move
  unsigned long nextStepMicros = micros();
  
  for (int i = 0; i < totalSteps; i++) {
    float currentVelocity; // steps/ms
    
    // Acceleration phase
    if (i < accelSteps) {
      float progress = (float)i / accelSteps;
      currentVelocity = V * progress;
    }
    // Cruise phase
    else if (i < accelSteps + cruiseSteps) {
      currentVelocity = V;
    }
    // Deceleration phase
    else {
      int decelStep = i - (accelSteps + cruiseSteps);
      float progress = 1.0 - ((float)decelStep / decelSteps);
      currentVelocity = V * progress;
    }
    
    // Calculate step period in microseconds
    unsigned long stepPeriodUs;
    if (currentVelocity > 0.0001) {
      stepPeriodUs = (unsigned long)(1000.0 / currentVelocity);
    } else {
      stepPeriodUs = 100000; // Very slow fallback
    }
    
    // Wait until it's time for the next step
    while (micros() < nextStepMicros);
    
    // Execute step
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_PULSE_US);
    digitalWrite(STEP_PIN, HIGH);
    
    nextStepMicros += stepPeriodUs;
  }
}


void loop() {
  // Example: 100 RPM max velocity, 3000ms total time, 1 revolution CW
  trapezoidalMove(150.0, 5000.0, 10.0);
  delay(1000);

  // -1 revolution = CCW (direction set automatically)
  trapezoidalMove(100.0, 3000.0, -10.0);
  delay(1000);
}
