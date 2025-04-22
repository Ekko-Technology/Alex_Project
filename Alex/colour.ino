#define signalOut PC7
#define NUM_READINGS 10  
#define SPEED_OF_SOUND 0.0345 // cm/us
#define TRIG_PIN PD7
#define ECHO_PIN PC1 

static uint32_t red, green, blue, avg_red, avg_green, avg_blue;

void setupColour()
{
    // Set S0, S1, S2, S3 to output on TCS230
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3); // corresponds to pins 53, 52, 51, 50
    // Set Out to input to capture signals
    DDRC &= ~(1 << PC7); // corresponds to pins 38

    // for ultrasonic sensor
    DDRC &= ~(1 << PC1); // Set ECHO pin to INPUT
    DDRD |= 0b10000000; // SET Trigger pin to OUTPUT


    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3); // Set S0, S1, S2, S3 to high
    PORTC |= (1 << PC7); // Enable internal pull-up resistor

    // setting frequency scaling to 20%, S0 = HIGH, S1 = LOW, ideal for using pulseIn
    PORTB |= (1 << PB0);  // S0 = HIGH
    PORTB &= ~(1 << PB1); // S1 = LOW
    
    // set trigger pin to low
    PORTD &= ~(1 << PD7); // Trigger pin LOW
}
long getPulse()
{
    long pulse = pulseIn(30, LOW); // 2-second timeout
    if (pulse == 0) {
        sendMessage("Error: No pulse detected!");
    }
    return pulse;
}

void detectColour()
{
    long total_red = 0, total_green = 0, total_blue = 0;

    // Select red filter
    PORTB &= ~(1 << PB2);
    PORTB &= ~(1 << PB3);
    sendMessage("Reading Red...");
    for (int i = 0; i < NUM_READINGS; i++) {
        total_red += getPulse();
    }
    avg_red = total_red / NUM_READINGS;

    // Select green filter
    PORTB |= (1 << PB2);
    PORTB |= (1 << PB3);
    sendMessage("Reading Green...");
    for (int i = 0; i < NUM_READINGS; i++) {
        total_green += getPulse();
    }
    avg_green = total_green / NUM_READINGS;

    // Select blue filter
    PORTB &= ~(1 << PB2);
    PORTB |= (1 << PB3);
    sendMessage("Reading Blue...");
    for (int i = 0; i < NUM_READINGS; i++) {
        total_blue += getPulse();
    }
    avg_blue = total_blue / NUM_READINGS;

    // Color detection
    TPacket colour_response = {0};
    colour_response.packetType = PACKET_TYPE_RESPONSE;
    colour_response.command = RESP_COLOUR;
    colour_response.params[0] = avg_red;
    colour_response.params[1] = avg_green;
    colour_response.params[2] = avg_blue;

    
    float total = avg_red + avg_green + avg_blue;
    float diff_green = 0;
    float diff_red = 0;
    

    float green_color[3] = {0.46592, 0.23110, 0.30298};
    float red_color[3] = {0.28301, 0.36436, 0.35263};

    for (int i = 0; i < 3; i++)
    {
      diff_green += ( (( (float) colour_response.params[i]/total) - green_color[i]) * (( (float) colour_response.params[i]/total) - green_color[i]) );
      diff_red += ( (( (float) colour_response.params[i]/total) - red_color[i]) * (( (float) colour_response.params[i]/total) - red_color[i]) );   
    }
    
    if (diff_red < diff_green)
    {
      sendMessage("Detected Color: Red");
      colour_response.data[0] = 'r';    
    }
    else
    {
        sendMessage("Detected Color: Green");
        colour_response.data[0] = 'g';
    }

    /*
    if ((avg_green > avg_red && avg_green > avg_blue) && !(avg_red < avg_green && avg_red < avg_blue)) {
        sendMessage("Detected Color: Green");
        colour_response.data[0] = 'g';
    } 
    else if (avg_red < avg_green && avg_red < avg_blue) {
        sendMessage("Detected Color: Red");
        colour_response.data[0] = 'r';
    } 
    else if ((avg_red + avg_green + avg_blue) < 600) {
        sendMessage("Detected Color: White");
        colour_response.data[0] = 'w';
    }
    else {
        sendMessage("Don't know color");
        colour_response.data[0] = 'u';
    }
    */
    detectDistance();
    // Send response
    sendResponse(&colour_response);
}

//void detectDistance()
//{
//    uint32_t duration = 0;
//    uint32_t distance = 0;
//    // Trigger ultrasonic pulse
//    PORTD |= (1 << PD7);
//    delayMicroseconds(10); // Wait for 10 microsec
//    PORTD &= ~(1 << PD7);
//
//    // Measure echo duration
//    duration = pulseIn(36, HIGH);
//    distance = (uint32_t)(duration * SPEED_OF_SOUND / 2.0);
//
//    // Prepare and send response
//    TPacket distance_response = {0};
//    distance_response.packetType = PACKET_TYPE_RESPONSE;
//    distance_response.command = RESP_ULTRASONIC;
//    distance_response.params[0] = distance;
//    distance_response.params[1] = duration;
//    distance_response.data[0] = 'd';
//
//    sendResponse(&distance_response);
//}

void detectDistance()
{
    uint32_t duration = 0;
    uint32_t distance = 0;

    // Clear Timer1
    TCCR1A = 0; // Normal mode
    TCCR1B = 0;
    TCNT1 = 0;

    // Trigger 10us pulse on PD7
    PORTD |= (1 << PD7);
    _delay_us(10);
    PORTD &= ~(1 << PD7);

    // Wait for echo pin (PC1) to go HIGH (start of echo)
    uint16_t timeout = 30000; // ~30ms timeout
    while (!(PINC & (1 << PC1)) && --timeout);

    if (timeout == 0) {
        sendMessage("Echo HIGH timeout");
        return;
    }

    // Start Timer1 with prescaler 8 (1 tick = 1 �s)
    TCCR1B |= (1 << CS11); // Prescaler = 8

    // Wait for echo pin to go LOW (end of echo)
    timeout = 60000; // ~60ms timeout
    while ((PINC & (1 << PC1)) && --timeout);

    // Stop Timer1
    TCCR1B = 0;

    if (timeout == 0) {
        sendMessage("Echo LOW timeout");
        return;
    }

    duration = TCNT1; // duration in �s
    distance = (uint32_t)(duration * SPEED_OF_SOUND / 2.0); // Convert to cm

    // Prepare and send response
    TPacket distance_response = {0};
    distance_response.packetType = PACKET_TYPE_RESPONSE;
    distance_response.command = RESP_ULTRASONIC;
    distance_response.params[0] = distance;
    distance_response.params[1] = duration;
    distance_response.data[0] = 'd';

    sendResponse(&distance_response);
}
