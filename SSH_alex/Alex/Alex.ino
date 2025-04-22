#include <math.h>
#include <stdarg.h>
#include <serialize.h>
#include "packet.h"
#include "constants.h"
#include "variables.h"

#define ALEX_LENGTH        26
#define ALEX_BREADTH       15

/*
 * Alex's configuration constants
 */

// Number of ticks per revolution from the 
// wheel encoder.

#define COUNTS_PER_REV      8

// Wheel circumference in cm.
// We will use this to calculate forward/backward distance traveled 
// by taking revs * WHEEL_CIRC

#define WHEEL_CIRC          21

/*
 *    Alex's State Variables
 */

unsigned long computeDeltaTicks(float ang){
  unsigned long ticks = (unsigned long) ((ang * alexCirc * COUNTS_PER_REV) / (360.0 * WHEEL_CIRC));
  return ticks;
}


/*
 * 
 * Alex Communication Routines.
 * 
 */

TResult readPacket(TPacket *packet)
{
	// Reads in data from the serial port and
	// deserializes it.Returns deserialized
	// data in "packet".

	char buffer[PACKET_SIZE];
	int len;

	len = readSerial(buffer);

	if(len == 0)
		return PACKET_INCOMPLETE;
	else
		return deserialize(buffer, len, packet);

}


void dbprintf(char *format, ...) {
	va_list args;
	char buffer[128];

	va_start(args, format);
	vsprintf(buffer, format, args);
	sendMessage(buffer);
}

/*
 * Setup and start codes for external interrupts and 
 * pullup resistors.
 * 
 */
// Enable pull up resistors on pins 18 and 19
void enablePullups()
{
	// Use bare-metal to enable the pull-up resistors on pins
	// 19 and 18. These are pins PD2 and PD3 respectively.
	// We set bits 2 and 3 in DDRD to 0 to make them inputs. 
	DDRD &= ~((1 << 2) | (1 << 3)); // Clear the relevant bits in DDRE to configure as input
	PORTD |= (1 << 2) | (1 << 3); // Setting these bits activates the built-in pull-up resistors, ensuring the line is held high when not actively pulled low by the sensor

}

// Functions to be called by INT2 and INT3 ISRs.
void leftISR()
{
	if (dir == FORWARD) {
		leftForwardTicks++;
		forwardDist = (unsigned long) ((float) leftForwardTicks / COUNTS_PER_REV * WHEEL_CIRC);
	} else if (dir == BACKWARD) {
		leftReverseTicks++;
		reverseDist = (unsigned long) ((float) leftReverseTicks / COUNTS_PER_REV * WHEEL_CIRC);
	} else if (dir == LEFT) {
		leftReverseTicksTurns++;
	} else if (dir == RIGHT) {
		leftForwardTicksTurns++;
	}
	//leftForwardTicks++;
	//Serial.print("LEFT: ");
	//Serial.println(leftForwardTicks);
}

void rightISR()
{
	if (dir == FORWARD) {
		rightForwardTicks++;
	} else if (dir == BACKWARD) {
		rightReverseTicks++;
	} else if (dir == LEFT) {
		rightForwardTicksTurns++;
	} else if (dir == RIGHT) {
		rightReverseTicksTurns++;
	}
	//rightForwardTicks++;
	//Serial.print("RIGHT: ");
	//Serial.println(rightForwardTicks);
}

// Implement the external interrupt ISRs below.
// INT3 ISR should call leftISR while INT2 ISR
// should call rightISR.

ISR(INT2_vect) {
	rightISR();
}

ISR(INT3_vect){
	leftISR();
}

// Implement INT2 and INT3 ISRs above.






/*
 * Setup and start codes for serial communications
 * 
 */
// Set up the serial connection. For now we are using 
// Arduino Wiring, you will replace this later
// with bare-metal code.
//void setupSerial()
//{
//	// To replace later with bare-metal.
//	Serial.begin(9600);
//	// Change Serial to Serial2/Serial3/Serial4 in later labs when using the other UARTs
//}
//
//// Start the serial connection. For now we are using
//// Arduino wiring and this function is empty. We will
//// replace this later with bare-metal code.
//
//void startSerial()
//{
//	// Empty for now. To be replaced with bare-metal code
//	// later on.
//
//}
//
//// Read the serial port. Returns the read character in
//// ch if available. Also returns TRUE if ch is valid. 
//// This will be replaced later with bare-metal code.
//
//int readSerial(char *buffer)
//{
//
//	int count=0;
//
//	// Change Serial to Serial2/Serial3/Serial4 in later labs when using other UARTs
//
//	while(Serial.available())
//		buffer[count++] = Serial.read();
//
//	return count;
//}
//
//// Write to the serial port. Replaced later with
//// bare-metal code
//
//void writeSerial(const char *buffer, int len)
//{
//	Serial.write(buffer, len);
//	// Change Serial to Serial2/Serial3/Serial4 in later labs when using other UARTs
//}



#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)

void setupSerial()
{
  // Configure baud rate
  UBRR0H = (uint8_t)(MYUBRR >> 8);
  UBRR0L = (uint8_t)(MYUBRR);

  // Enable receiver and transmitter
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);

  // Set frame format: 8 data bits, no parity, 1 stop bit (8N1)
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void startSerial()
{
  // No need for extra start logic for UART0 in bare-metal
}

int readSerial(char *buffer)
{
  int count = 0;

  // While data is available
  while (UCSR0A & (1 << RXC0))
  {
    buffer[count++] = UDR0;
  }

  return count;
}

void writeSerial(const char *buffer, int len)
{
  for (int i = 0; i < len; i++)
  {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));

    // Put data into buffer, sends the data
    UDR0 = buffer[i];
  }
}






/*
 * Alex's setup and run codes
 * 
 */

// Clears all our counters
void clearCounters()
{
	leftForwardTicks=0;
	rightForwardTicks=0;
	leftReverseTicks=0;
	rightReverseTicks=0;
	leftForwardTicksTurns=0;
	rightForwardTicksTurns=0;
	leftReverseTicksTurns=0;
	rightReverseTicksTurns=0;
	leftRevs=0;
	rightRevs=0;
	forwardDist=0;
	reverseDist=0; 
}

// Clears one particular counter
void clearOneCounter(int which)
{
	clearCounters();
}
// Intialize Alex's internal states

void initializeState()
{
	clearCounters();
}


void waitForHello()
{
	int exit=0;

	while(!exit)
	{
		TPacket hello;
		TResult result;

		do
		{
			result = readPacket(&hello);
		} while (result == PACKET_INCOMPLETE);

		if(result == PACKET_OK)
		{
			if(hello.packetType == PACKET_TYPE_HELLO)
			{
				sendOK();
				exit=1;
			}
			else
				sendBadResponse();
		}
		else
			if(result == PACKET_BAD)
			{
				sendBadPacket();
			}
			else
				if(result == PACKET_CHECKSUM_BAD)
					sendBadChecksum();
	} // !exit
}

void setup() {
	// put your setup code here, to run once:
	alexDiagonal = sqrt((ALEX_LENGTH * ALEX_LENGTH) + (ALEX_BREADTH * ALEX_BREADTH));
	alexCirc = PI * alexDiagonal;
	cli();
	setupEINT();
	setupSerial();
	startSerial();
	enablePullups();
	initializeState();
  setup_claw();
  setupColour();
	sei();
}

void handlePacket(TPacket *packet)
{
	switch(packet->packetType)
	{
		case PACKET_TYPE_COMMAND:
			handleCommand(packet);
			break;

		case PACKET_TYPE_RESPONSE:
			break;

		case PACKET_TYPE_ERROR:
			break;

		case PACKET_TYPE_MESSAGE:
			break;

		case PACKET_TYPE_HELLO:
			break;
	}
}

void loop() {
	// Uncomment the code below for Step 2 of Activity 3 in Week 8 Studio 2

	// forward(0, 100);

	// Uncomment the code below for Week 9 Studio 2


	// put your main code here, to run repeatedly:
	TPacket recvPacket; // This holds commands from the Pi

	TResult result = readPacket(&recvPacket); // Reads from serial port?

	if(result == PACKET_OK)
		handlePacket(&recvPacket);
	else
		if(result == PACKET_BAD)
		{
			sendBadPacket();
		}
		else
			if(result == PACKET_CHECKSUM_BAD)
			{
				sendBadChecksum();
			} 

	if(deltaDist > 0) {
		if(dir==FORWARD)
		{
			if(forwardDist > newDist)
			{
				deltaDist=0;
				newDist=0;
				stop();
			}
		}
		else
			if(dir == BACKWARD)
			{
				if(reverseDist > newDist)
				{
					deltaDist=0;
					newDist=0;
					stop();
				}
			}
			else

				if(dir == STOP)
				{
					deltaDist=0;
					newDist=0;
					stop();
				}
	}

	if(deltaTicks > 0) {
		if(dir==LEFT)
		{
			if(leftReverseTicksTurns >= targetTicks)
			{
				deltaTicks=0;
				targetTicks=0;
				stop();
			}
		}
		else
			if(dir == RIGHT)
			{
				if(leftForwardTicksTurns >= targetTicks)
				{
					deltaTicks=0;
					targetTicks=0;
					stop();
				}
			}
			else
				if(dir == STOP)
				{
					deltaTicks=0;
					targetTicks=0;
					stop();
				}
	}
}
