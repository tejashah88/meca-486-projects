// aaron_stepper.h
// Green, Aaron
// asgreen

#ifndef AARON_STEPPER_H
#define AARON_STEPPER_H

#include <Arduino.h>

class Aaron_Stepper {
private:
    // Pin configuration
    int enable_side_a_pin;
    int enable_side_b_pin;
    int input_pins[4];
    
    // Motor configuration
    int steps_per_rev;
    int rpm;
    unsigned long step_delay_us;
    int enable_duty_cycle;
    
    // Position tracking
    int internal_step_position;  // 0-3 for 4-step sequence
    long global_step_counter;    // Total steps from initialization
    
    // Step sequence pattern
    static const int DRIVER_STEPS[4][4];
    
    // Internal helper method
    void stepToIthPosition(int i);
    
public:
    // Constructor
    Aaron_Stepper(int en_a, int en_b, int in1, int in2, int in3, int in4, 
                  int steps_per_rev = 200, int initial_rpm = 100, int duty_cycle = 51);
    
    // Initialization
    void begin();
    
    // Movement methods
    void stepNForward(int n);
    void stepNBackward(int n);
    void moveSteps(int steps);  // Positive = forward, negative = backward
    
    // Configuration
    void setRPM(int rpm);
    void setDutyCycle(int duty_cycle);
    
    // Control
    void enable();
    void disable();
    
    // Position tracking
    long getStepCounter() const;
    int getInternalPosition() const;
    void resetStepCounter();
    
    // Advanced movement
    void trapMove(unsigned long time_sec, int revs, int max_vel);
};

#endif
