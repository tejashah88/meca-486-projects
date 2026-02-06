#include "aaron_stepper.h"

// Define the static step sequence
const int Aaron_Stepper::DRIVER_STEPS[4][4] = {
  {HIGH, LOW,  HIGH, LOW},
  {LOW,  HIGH, HIGH, LOW},
  {LOW,  HIGH, LOW,  HIGH},
  {HIGH, LOW,  LOW,  HIGH}
};

// Constructor
Aaron_Stepper::Aaron_Stepper(int en_a, int en_b, int in1, int in2, int in3, int in4, 
                             int steps_per_rev, int initial_rpm, int duty_cycle) {
    enable_side_a_pin = en_a;
    enable_side_b_pin = en_b;
    input_pins[0] = in1;
    input_pins[1] = in2;
    input_pins[2] = in3;
    input_pins[3] = in4;
    
    this->steps_per_rev = steps_per_rev;
    this->rpm = initial_rpm;
    this->enable_duty_cycle = duty_cycle;
    
    // Calculate initial step delay
    step_delay_us = 60000000UL / ((unsigned long)rpm * steps_per_rev);
    
    // Initialize position tracking
    internal_step_position = 0;
    global_step_counter = 0;
}

void Aaron_Stepper::begin() {
    // Setup enable pins
    pinMode(enable_side_a_pin, OUTPUT);
    pinMode(enable_side_b_pin, OUTPUT);
    
    // Setup input pins
    for (int i = 0; i < 4; i++) {
        pinMode(input_pins[i], OUTPUT);
    }
    
    // Set initial position to step 0
    digitalWrite(input_pins[0], DRIVER_STEPS[0][0]);
    digitalWrite(input_pins[1], DRIVER_STEPS[0][1]);
    digitalWrite(input_pins[2], DRIVER_STEPS[0][2]);
    digitalWrite(input_pins[3], DRIVER_STEPS[0][3]);
    
    // Enable the motor
    enable();
}

void Aaron_Stepper::stepToIthPosition(int i) {
    digitalWrite(input_pins[0], DRIVER_STEPS[i][0]);
    digitalWrite(input_pins[1], DRIVER_STEPS[i][1]);
    digitalWrite(input_pins[2], DRIVER_STEPS[i][2]);
    digitalWrite(input_pins[3], DRIVER_STEPS[i][3]);
    delayMicroseconds(step_delay_us);
}

void Aaron_Stepper::stepNForward(int n) {
    int j = internal_step_position;
    for (int i = 0; i < n; i++) {
        j = (j + 1) & 0x03;  // Wrap 0-3
        stepToIthPosition(j);
    }
    internal_step_position = j;
    global_step_counter += n;
}

void Aaron_Stepper::stepNBackward(int n) {
    int j = internal_step_position;
    for (int i = 0; i < n; i++) {
        j = (j - 1) & 0x03;  // Wrap 0-3
        stepToIthPosition(j);
    }
    internal_step_position = j;
    global_step_counter -= n;
}

void Aaron_Stepper::moveSteps(int steps) {
    if (steps < 0) {
        stepNBackward(-steps);
    } else {
        stepNForward(steps);
    }
}

void Aaron_Stepper::setRPM(int new_rpm) {
    rpm = new_rpm;
    step_delay_us = 60000000UL / ((unsigned long)rpm * steps_per_rev);
}

void Aaron_Stepper::setDutyCycle(int duty_cycle) {
    enable_duty_cycle = duty_cycle;
    if (duty_cycle > 0) {
        analogWrite(enable_side_a_pin, duty_cycle);
        analogWrite(enable_side_b_pin, duty_cycle);
    }
}

void Aaron_Stepper::enable() {
    analogWrite(enable_side_a_pin, enable_duty_cycle);
    analogWrite(enable_side_b_pin, enable_duty_cycle);
}

void Aaron_Stepper::disable() {
    analogWrite(enable_side_a_pin, 0);
    analogWrite(enable_side_b_pin, 0);
}

long Aaron_Stepper::getStepCounter() const {
    return global_step_counter;
}

int Aaron_Stepper::getInternalPosition() const {
    return internal_step_position;
}

void Aaron_Stepper::resetStepCounter() {
    global_step_counter = 0;
}

void Aaron_Stepper::trapMove(unsigned long time_sec, int revs, int max_vel) {
    // Using formula from course materials: t_a = t_m - X/V
    // where t_m = total move time, X = distance (revs), V = max velocity (RPM)
    
    // Calculate total steps needed
    long total_steps = (long)revs * steps_per_rev;
    
    // Calculate acceleration time in microseconds
    // t_a = t_m - X/V, where X/V gives time if traveling at constant max velocity
    unsigned long t_m_us = time_sec * 1000000UL;
    unsigned long constant_vel_time_us = ((unsigned long)revs * 60UL * 1000000UL) / max_vel;
    
    // Total accel + decel time (symmetric, so each gets half)
    long accel_decel_time_us = t_m_us - constant_vel_time_us;
    
    if (accel_decel_time_us < 0) {
        // Cannot reach max velocity in given time - triangular profile
        accel_decel_time_us = t_m_us;
        constant_vel_time_us = 0;
    }
    
    unsigned long t_a_us = accel_decel_time_us / 2;  // Acceleration time
    unsigned long t_d_us = accel_decel_time_us / 2;  // Deceleration time (symmetric)
    unsigned long t_c_us = constant_vel_time_us;     // Cruise time
    
    // Calculate acceleration (in RPM/us)
    // a = V / t_a
    float acceleration = (float)max_vel / (float)t_a_us;  // RPM per microsecond
    
    // Calculate steps in each phase using area under velocity curve
    // Accel phase: distance = (1/2) * V * t_a (triangle area)
    long accel_steps = ((long)max_vel * t_a_us) / (60UL * 1000000UL * 2UL) * steps_per_rev;
    long decel_steps = accel_steps;  // Symmetric
    long cruise_steps = total_steps - accel_steps - decel_steps;
    
    // Calculate step delay at max velocity
    unsigned long step_delay_max_us = 60UL * 1000000UL / ((unsigned long)max_vel * steps_per_rev);
    
    // Phase 1: Accelerate (linear ramp from 0 to max_vel)
    unsigned long accel_start = micros();
    while (accel_steps > 0) {
        unsigned long elapsed = micros() - accel_start;
        if (elapsed >= t_a_us) break;
        
        // Current velocity: v(t) = a * t
        float current_rpm = acceleration * elapsed;
        if (current_rpm < 1.0) current_rpm = 1.0;  // Minimum speed
        
        step_delay_us = 60UL * 1000000UL / ((unsigned long)current_rpm * steps_per_rev);
        stepNForward(1);
        accel_steps--;
    }
    
    // Phase 2: Cruise at constant max velocity
    step_delay_us = step_delay_max_us;
    unsigned long cruise_start = micros();
    while (cruise_steps > 0) {
        unsigned long elapsed = micros() - cruise_start;
        if (elapsed >= t_c_us) break;
        
        stepNForward(1);
        cruise_steps--;
    }
    
    // Phase 3: Decelerate (linear ramp from max_vel to 0)
    unsigned long decel_start = micros();
    while (decel_steps > 0) {
        unsigned long elapsed = micros() - decel_start;
        if (elapsed >= t_d_us) break;
        
        // Current velocity: v(t) = V - a * t (ramp down)
        float current_rpm = max_vel - (acceleration * elapsed);
        if (current_rpm < 1.0) current_rpm = 1.0;  // Minimum speed
        
        step_delay_us = 60UL * 1000000UL / ((unsigned long)current_rpm * steps_per_rev);
        stepNForward(1);
        decel_steps--;
    }
    
    // Restore original RPM setting
    setRPM(rpm);
}
