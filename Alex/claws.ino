// we will have to test this out to record the true range values
// we will use 16-bit timer's pin 45(PL4) and 46(PL3) which corresponds to 0C5B and 0C5A counters respectively
static long left_servo_open = 285;
static long right_servo_open = 1200;
static long left_servo_close = 600;
static long right_servo_close = 900;
static long state = 1;

void setup_claw() {
    // Set PL3 (OC5A) and PL4 (OC5B) as outputs
    DDRL |= (1 << PL3) | (1 << PL4);

    // Configure Timer5 for Phase and Frequency Correct PWM
    TCCR5A = (1 << COM5A1) | (1 << COM5B1); // Enable OC5A and OC5B outputs
    TCCR5B = (1 << WGM53) | (1 << CS51);    // Mode 8, prescaler = 8

    ICR5 = 20000; // Set TOP value for 50Hz frequency

    // Initialize servos to open position
    OCR5A = left_servo_open;
    OCR5B = right_servo_open;
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
