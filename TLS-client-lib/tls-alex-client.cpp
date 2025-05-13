#include <ctype.h>
// Routines to create a TLS client
#include "../libraries/make_tls_client.h"

// Network packet types
#include "../libraries/netconstants.h"

// Packet types, error codes, etc.
#include "../libraries/constants.h"

// Tells us that the network is running.
static volatile int networkActive=0;

void handleError(const char *buffer)
{
	switch(buffer[1])
	{
		case RESP_OK:
			printf("Command / Status OK\n");
			break;

		case RESP_BAD_PACKET:
			printf("BAD MAGIC NUMBER FROM ARDUINO\n");
			break;

		case RESP_BAD_CHECKSUM:
			printf("BAD CHECKSUM FROM ARDUINO\n");
			break;

		case RESP_BAD_COMMAND:
			printf("PI SENT BAD COMMAND TO ARDUINO\n");
			break;

		case RESP_BAD_RESPONSE:
			printf("PI GOT BAD RESPONSE FROM ARDUINO\n");
			break;

		default:
			printf("PI IS COBNFUSED!\n");
	}
}

void handleStatus(const char *buffer)
{
	int32_t data[16];
	memcpy(data, &buffer[1], sizeof(data));

	printf("\n ------- ALEX STATUS REPORT ------- \n\n");
	printf("Left Forward Ticks:\t\t%d\n", data[0]);
	printf("Right Forward Ticks:\t\t%d\n", data[1]);
	printf("Left Reverse Ticks:\t\t%d\n", data[2]);
	printf("Right Reverse Ticks:\t\t%d\n", data[3]);
	printf("Left Forward Ticks Turns:\t%d\n", data[4]);
	printf("Right Forward Ticks Turns:\t%d\n", data[5]);
	printf("Left Reverse Ticks Turns:\t%d\n", data[6]);
	printf("Right Reverse Ticks Turns:\t%d\n", data[7]);
	printf("Forward Distance:\t\t%d\n", data[8]);
	printf("Reverse Distance:\t\t%d\n", data[9]);
	if (data[10]){
		printf("Claw OPEN\n");
	}
	else{
		printf("Claw CLOSED\n");
	}
	if(data[11]){
		printf("Trapdoor CLOSED\n");
	}
	else{
		printf("Trapdoor OPEN\n");
	}
	printf("\n---------------------------------------\n\n");
}

void handleUltrasonic(const char *buffer)
{
	int32_t data[16];
	memcpy(data, &buffer[1], sizeof(data));
	printf("Ultrasonic Distance: %d cm\n", data[0]);
}

void handleColour(const char *buffer)
{
	char colour = buffer[1];
	if (colour == 'r')
	{
		printf("Colour Sensor: Red\n");
	}
	else if (colour == 'g')
	{
		printf("Colour Sensor: Green\n");
	}
	else if (colour == 'w')
	{
		printf("Colour Sensor: White\n");
	}
	else
	{
		printf("Unknown Colour Detected\n");
	}
	int32_t data[16];
	memcpy(data, &buffer[2], sizeof(data));
	printf("Red: %d\n", data[0]);
	printf("Green: %d\n", data[1]);
	printf("Blue: %d\n", data[2]);
}

void handleMessage(const char *buffer)
{
	printf("MESSAGE FROM ALEX: %s\n", &buffer[1]);
}

void handleCommand(const char *buffer)
{
	// We don't do anything because we issue commands
	// but we don't get them. Put this here
	// for future expansion
	switch(buffer[1]){
		case NET_COLOUR:
			printf("Colour Sensor Data: %d\n", *(int32_t *)&buffer[2]);
			break;
		case NET_ULTRASONIC:
			handleUltrasonic(buffer);
			break;
		default:
			printf("Unknown command from server: %d\n", buffer[1]);
			break;
	}	
}

void handleNetwork(const char *buffer, int len)
{
	// The first byte is the packet type
	int type = buffer[0];

	switch(type)
	{
		case NET_ERROR_PACKET:
		handleError(buffer);
		break;

		case NET_STATUS_PACKET:
		handleStatus(buffer);
		break;

		case NET_MESSAGE_PACKET:
		handleMessage(buffer);
		break;

		case NET_COMMAND_PACKET:
		handleCommand(buffer);
		break;

		case NET_ULTRASONIC:
		handleUltrasonic(buffer);
		break;

		case NET_COLOUR:
		handleColour(buffer);
		break;
	}
}

void sendData(void *conn, const char *buffer, int len)
{
	int c;
	printf("\nSENDING %d BYTES DATA\n\n", len);
	if(networkActive)
	{
		/* TODO: Insert SSL write here to write buffer to network */
		c = sslWrite(conn, buffer, len);

		/* END TODO */	
		networkActive = (c > 0);
	}
}
void receiveData(void *conn, char *buffer, int bufferSize)
{
    int bytesRead;
    printf("\nWAITING TO RECEIVE DATA...\n\n");

    if (networkActive)
    {
        /* TODO: Insert SSL read here to read data i				 buffer */
        bytesRead = sslRead(conn, buffer, bufferSize);

        /* END TODO */
        if (bytesRead > 0)
        {
            printf("RECEIVED %d BYTES DATA\n\n", bytesRead);
            handleNetwork(buffer, bytesRead); // Process the received data
        }
        else
        {
            printf("FAILED TO RECEIVE DATA OR CONNECTION CLOSED\n");
            networkActive = 0; // Mark the network as inactive
        }
    }
}

void *readerThread(void *conn)
{
	char buffer[128];
	int len;

	while(networkActive)
	{
		/* TODO: Insert SSL read here into buffer */

		len = sslRead(conn, buffer, 128);
        printf("read %d bytes from server.\n", len);
		
		/* END TODO */

		networkActive = (len > 0);

		if(networkActive)
			handleNetwork(buffer, len);
	}

	printf("Exiting network listener thread\n");
    
    /* TODO: Stop the client loop and call EXIT_THREAD */

	EXIT_THREAD(conn);
    /* END TODO */

    return NULL;
}

void flushInput()
{
	char c;

	while((c = getchar()) != '\n' && c != EOF);
}

void getParams(int32_t *params)
{
	printf("Enter Angle in degrees in %%.\n");
	do{
		scanf("%d", &params[0]);
	} while(params[0] < 20 || params[0] > 220);
	flushInput();
}

void print_control_table(){
  printf("+------------------+--------------------+\n");
  printf("| Key              | Action             |\n");
  printf("+------------------+--------------------+\n");
  printf("| W                | Forward (Short)    |\n");
  printf("| S                | Reverse (Short)    |\n");
  printf("| P                | Forward (Long)     |\n");
  printf("| L                | Reverse (Long)     |\n");
  printf("| A                | Left               |\n");
  printf("| D                | Right              |\n");
  printf("| T                | E-Stop             |\n");
  printf("+------------------+--------------------+\n");
  printf("| X                | Colour Detection   |\n");
  printf("| M                | Control Arm        |\n");
  printf("| U                | Front Distance     |\n");
  printf("| J                | Open Trapdoor      |\n");
  printf("+------------------+--------------------+\n");
  printf("| C                | Clear Stats        |\n");
  printf("| G                | Get Stats          |\n");
  printf("| Q                | Quit               |\n");
  printf("+------------------+--------------------+\n");
}

void *writerThread(void *conn)
{
	int quit=0;

	while(!quit)
	{
		char ch;
    	print_control_table();
		scanf("%c", &ch);
    	ch = tolower(ch);

		// Purge extraneous characters from input stream
		flushInput();

		char buffer[10];
		int32_t params[] = {0,0};

		buffer[0] = NET_COMMAND_PACKET;
		switch(ch)
		{
      		//Motor
			case 'w':
			case 's':
			case 'p':
			case 'l':
				buffer[1] = ch;
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;
			case 'a':
			case 'd':
				getParams(params);
				if (ch == 'd'){
					params[0] = params[0]*0.66667; // Convert to ticks
				}
				buffer[1] = ch;
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;
			
			case 'x': // Color Detection
			{
				buffer[1] = ch; // Command for color sensor
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer)); // Send the request
				break;
			}
			case 'u': // Ultrasonic Distance
			{
				buffer[1] = ch; // Command for ultrasonic sensor
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer)); // Send the request
				break;
			}
			//Arm Control
			case 'm':
			case 'j':
			case 't':
			case 'c':
			case 'g':
				memcpy(&buffer[2], params, sizeof(params));
				buffer[1] = ch;
				sendData(conn, buffer, sizeof(buffer));
				break;
			case 'q':
				quit=1;
				break;
			default:
				printf("BAD COMMAND\n");
		}
	}

	printf("Exiting keyboard thread\n");

    /* TODO: Stop the client loop and call EXIT_THREAD */
	EXIT_THREAD(conn);

    /* END TODO */

    return NULL;
}

/* TODO: #define filenames for the client private key, certificatea,
   CA filename, etc. that you need to create a client */
#define SERVER_NAME "192.168.178.239"
#define CA_CERT_FNAME "TLS-client-lib/signing.pem"
#define PORT_NUM 5001
#define CLIENT_CERT_FNAME "TLS-client-lib/laptop.crt"
#define CLIENT_KEY_FNAME "TLS-client-lib/laptop.key"
#define SERVER_NAME_ON_CERT "epp_pi.com"

/* END TODO */
void connectToServer(const char *serverName, int portNum)
{
    /* TODO: Create a new client */
	createClient(SERVER_NAME, PORT_NUM, 1, CA_CERT_FNAME, SERVER_NAME_ON_CERT, 1, CLIENT_CERT_FNAME, CLIENT_KEY_FNAME, readerThread, writerThread);

    /* END TODO */
}

int main(int ac, char **av)
{
	if(ac != 3)
	{
		fprintf(stderr, "\n\n%s <IP address> <Port Number>\n\n", av[0]);
		exit(-1);
	}

    networkActive = 1;
    connectToServer(av[1], atoi(av[2]));

    /* TODO: Add in while loop to prevent main from exiting while the
    client loop is running */

	while(client_is_running());

    /* END TODO */
	printf("\nMAIN exiting\n\n");
}
