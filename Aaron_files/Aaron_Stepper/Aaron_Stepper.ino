// Hardware pin setup

const int ENABLE_SIDE_A_PIN = 10;
const int ENABLE_SIDE_B_PIN = 9;

const int INPUT_PIN_1 = 7;
const int INPUT_PIN_2 = 6;
const int INPUT_PIN_3 = 5;
const int INPUT_PIN_4 = 4;

// Stepper motor control

// Using 20% out of maximum 29.4% duty cycle (30 V nominal) for current limiting
// Measured voltage from AMP PS430 is 38 V, thus maximum is 22% smth
// L298N can only handle maximum 25 W
const int ENABLE_DUTY_CYCLE_AMT = (int) (0.20 * 255);

const int STEPS_PER_REV = 200;
int RPM = 100;
unsigned long STEP_DELAY_US = 60000000UL / ((unsigned long)RPM * STEPS_PER_REV);//using microseconds and unsigned long for more precision. Not using double or floating point

const int DRIVER_STEPS[4][4] = {
  {HIGH, LOW,  HIGH, LOW},
  {LOW,  HIGH, HIGH, LOW},
  {LOW,  HIGH, LOW,  HIGH},
  {HIGH, LOW,  LOW,  HIGH}
};

// Global variables for internal position pseudo-recording
int Global_step_counter = 0;
int internal_step_position = 0;

//

void stepToIthPosition(int i) {
  digitalWrite(INPUT_PIN_1, DRIVER_STEPS[i][0]);
  digitalWrite(INPUT_PIN_2, DRIVER_STEPS[i][1]);
  digitalWrite(INPUT_PIN_3, DRIVER_STEPS[i][2]);
  digitalWrite(INPUT_PIN_4, DRIVER_STEPS[i][3]);
  delayMicroseconds(STEP_DELAY_US);
}

void stepNForward(int n) { //number of steps
  int j = internal_step_position;
  for(int i = 0; i < n; i++){
    j = (j + 1) & 0x03;// does base 2 and operation
    stepToIthPosition(j);
  }
  internal_step_position = j;
  }
void stepNBackward(int n) { //number of steps
  int j = internal_step_position;
  for(int i = 0; i < n; i++){
    j = (j - 1) & 0x03;// wraps to (111111) base 2
    stepToIthPosition(j);
  }
  internal_step_position = j;
  }

void move_steps(int x){
  if(x<0){
    stepNBackward(-x);
  }
  else{ stepNForward(x);}
  Global_step_counter += x;

}



void setRPM(int n){
  RPM = n;
 STEP_DELAY_US = 60000000UL / ((unsigned long)RPM * STEPS_PER_REV);//UL means unsigned long for the int
}

void disable(){
  analogWrite(ENABLE_SIDE_A_PIN, 0);
  analogWrite(ENABLE_SIDE_B_PIN, 0);
}

void trap_move(unsigned long time_sec, int revs, int max_vel){
unsigned long ta_us = time_sec*1000000UL - ((unsigned long)revs*1000000UL*60/ ((unsigned long)max_vel));
int tmid_us = time_sec*1000000UL - (ta_us * 2);
int acceleration =  ((unsigned long)max_vel /60 /1000000UL)/ ta_us; // rev/us^2

unsigned long start_time = micros();


//accelerate
while((unsigned long elapsed_time = (micros() - start_time))< ta_us){
  unsigned long RPUS = acceleration*elapsed_time;
  STEP_DELAY_US = 1UL / ((unsigned long)RPUS * STEPS_PER_REV);  
 stepNForward(1);
}

//Mid section
start_time = micros();
while((micros() - startime)< tmid_us){

  setRPM(max_vel);
  stepNForward(1);
}


//slowdown
while((unsigned long elapsed_time = (micros() - start_time))< ta_us){
  unsigned long RPUS = ((unsigned long)max_vel /60 /1000000UL) - (acceleration*elapsed_time);
  STEP_DELAY_US = 1UL / ((unsigned long)RPUS * STEPS_PER_REV) ; 
  stepNForward(1);
}


}



void setup() {
  // Setup up and enable PWM pins for power modulating the motor driver board (enable pins)
  pinMode(ENABLE_SIDE_A_PIN, OUTPUT);
  pinMode(ENABLE_SIDE_B_PIN, OUTPUT);

  analogWrite(ENABLE_SIDE_A_PIN, ENABLE_DUTY_CYCLE_AMT);
  analogWrite(ENABLE_SIDE_B_PIN, ENABLE_DUTY_CYCLE_AMT);

  // Configure input pins to control stepper motor
  pinMode(INPUT_PIN_1, OUTPUT);
  pinMode(INPUT_PIN_2, OUTPUT);
  pinMode(INPUT_PIN_3, OUTPUT);
  pinMode(INPUT_PIN_4, OUTPUT);

  // Set start of stepper motor to 1 of 50 known positions and set the internal step counter to 0 
  digitalWrite(INPUT_PIN_1, DRIVER_STEPS[0][0]);
  digitalWrite(INPUT_PIN_2, DRIVER_STEPS[0][1]);
  digitalWrite(INPUT_PIN_3, DRIVER_STEPS[0][2]);
  digitalWrite(INPUT_PIN_4, DRIVER_STEPS[0][3]);
  
  Global_step_counter = 0;
  internal_step_position = 0;

  delay(1000);
  move_steps(800);
  setRPM(60);
  move_steps(-400);
  disable();



}

void loop() {
  
}
