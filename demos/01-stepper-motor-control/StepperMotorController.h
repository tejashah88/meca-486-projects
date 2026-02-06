#ifndef STEPPER_MOTOR_CONTROLLER
#define STEPPER_MOTOR_CONTROLLER

class StepperMotorController
{
  public:
    // Constructor for hardware initialization
    StepperMotor(
      int _enablePin_A, int _pin_A_Plus, int _pin_A_Minus,
      int _enablePin_B, int _pin_B_Plus, int _pin_B_Minus,
    );

    // Dynamic configuration setters
    void setSteppingMode();
    void setStepsPerRev(int _stepsPerRev) { stepsPerRev = _stepsPerRev; };
    void setRevsPerMin(int _revsPerMin)   { revsPerMin = _revsPerMin; };
  private:
    // Side A hardware pins
    int enablePin_A;
    int pin_A_Plus;
    int pin_A_Minus;

    // Side B hardware pins
    int enablePin_B;
    int pin_B_Plus;
    int pin_B_Minus;

    // Driver steps table for half stepping

    // Columns: A, A_not, B, B_not
    // Diagram ordering: A, B, A_not, B_not
    const int DRIVER_HALF_STEPS[8][4] = {
      {HIGH, LOW,  LOW,  LOW},    // A
      {HIGH, LOW,  HIGH, LOW},    // A, B
      {LOW,  LOW,  HIGH, LOW},    // B
      {LOW,  HIGH, HIGH, LOW},    // B, A_not
      {LOW,  HIGH, LOW,  LOW},    // A_not
      {LOW,  HIGH, LOW,  HIGH},   // A_not, B_not
      {LOW,  LOW,  LOW,  HIGH},   // B_not
      {HIGH, LOW,  LOW,  HIGH},   // B_not, A
    };

    // Indirect motor controller parameters
    unsigned long stepsPerRev;
    unsigned long revsPerMin;

    //
    unsigned long _calculateStepDelayUs() { return (60 000 000UL / (unsigned long) (revsPerMin * stepsPerRev); };
};

#endif STEPPER_MOTOR_CONTROLLER
