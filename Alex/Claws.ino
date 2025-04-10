// we will have to test this out to record the true range values
// we will use 16-bit timer's pin 45(PL4) and 46(PL3) which corresponds to 0C5B and 0C5A counters respectively
static long left_servo_open = 1000;
static long right_servo_open = 500;
static long left_servo_close = 2000;
static long right_servo_close = 1500;

void setup()
{
    // PB5 links OC1A/pin 24 and 
    DDRL |= (1 << PL3) | (1 << PL4); // Set OC1A as output

    TCCR5A = 0b10000010; // Clear OC1A on compare match

    TCCR5B = 0b00010010; // prescaler = 8 / phase correct PWM

    // we will have to set this to Duty cycle of opening both claws
    OCR5A = 1000; // duty cycle = 5%, determines the position of the servo
    OCR5B = 500; 

    ICR5 = 20000; // TOP value, counts up from 
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
