// we will have to test this out to record the true range values
// we will use 16-bit timer's pin 45(PL4) and 46(PL3) which corresponds to 0C5B and 0C5A counters respectively
static long left_servo_open = 280;
static long right_servo_open = 1200;
static long left_servo_close = 500;
static long right_servo_close = 700;
static long state = 1;
static long trap_state = 1;

static long trapdoor_open = 2000;
static long trapdoor_close = 800;

void setup_claw() {
    // Set PL3 (OC5A) and PL4 (OC5B) as outputs
    DDRL |= (1 << PL3) | (1 << PL4) | (1 << PL5);

    // Configure Timer5 for Phase and Frequency Correct PWM
    TCCR5A = (1 << COM5A1) | (1 << COM5B1) | (1 << COM5C1); // Enable OC5A, OC5B and OC5C outputs
    TCCR5B = (1 << WGM53) | (1 << CS51);    // Mode 8, prescaler = 8

    ICR5 = 20000; // Set TOP value for 50Hz frequency

    // Initialize servos to open position
    OCR5A = left_servo_open;
    OCR5B = right_servo_open;
    OCR5C = trapdoor_close;

}


void claws_open()
{
  OCR5A = left_servo_open; 
  OCR5B = right_servo_open; 
}

void claws_close()
{
  OCR5A = left_servo_close;
  OCR5B = right_servo_close;
}

void toggle_claw()
{
  if (state)
  {
    claws_close();
  }
  else
  {
    claws_open();
  }
  state = 1 - state;
}

void toggle_trapdoor(){
  if(trap_state){
    OCR5C = trapdoor_open;
  }
  else{
    OCR5C = trapdoor_close;
  }
  trap_state = 1 - trap_state;

}
