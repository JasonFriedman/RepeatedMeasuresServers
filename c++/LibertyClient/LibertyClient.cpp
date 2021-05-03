#include "LibertyClient.h"
#include "messagecodes.h"

int setupdevice(SOCKET ConnectSocket, int numSensors) {

	int samplerate = 240;

	// Set the mode to binary
	printf("Setting mode to binary\n");
	setMode(ConnectSocket,1);
	Sleep(300);

	// Set the sample rate to 240 Hz
	printf("Setting sample rate to 240 Hz\n");
	setsamplerate(ConnectSocket,samplerate);
	Sleep(300);

	//We get the update rate as a way of checking that the liberty is still
	// answering us (and not stuck running some command or error)

	if(getupdaterate(ConnectSocket) != samplerate) {
		printf("Problem after setsamplerate\n");
		return -1;
	}

	// set the units to cm
	printf("Setting units to cm\n");
	setunits(ConnectSocket,1);
	Sleep(300);
	if(getupdaterate(ConnectSocket) != samplerate) {
		printf("Problem after setunits\n");
		return -1;
	}

	// set the active hemisphere
	for (int k=0;k<numSensors;k++) {
		printf("Setting hemisphere for %d to 0,1,0\n",k);
		sethemisphere(ConnectSocket,k,0,1,0);
		Sleep(100);
		if(getupdaterate(ConnectSocket) != samplerate) {
		printf("Problem after sethemisphere\n");
		return -1;
		}
	}

	// Reset the frame count
	printf("Resetting the frame count\n");
	resetframecount(ConnectSocket);
	Sleep(300);

	if(getupdaterate(ConnectSocket) != samplerate) {
		printf("Problem after resetframecount\n");
	}

	// set the output format
	for (int k=0;k<numSensors;k++) {
		setoutputformat(ConnectSocket,k,1);
		Sleep(100);
		if(getupdaterate(ConnectSocket) != samplerate) {
			printf("Problem after setoutputformat\n");
			return -1;
		}	
	}


	// set the alignment reference frame (which way are origin (3 numbers), x axis (3 numbers), y axis(3 numbers))
	for (int k=0;k<numSensors;k++) {
		setalignmentframe(ConnectSocket,k,
			0,0,0,1,0,0,0,1,0);
		Sleep(100);
		if(getupdaterate(ConnectSocket) != samplerate) {
			printf("Problem after setalignmentframe\n");
			return -1;
		}
	}

	// get a sample to check everything is working
	double result[500];
	getsinglesample(ConnectSocket,numSensors,result);
	if (result[0]==0) {
		printf("Did not receive a result from the server - check everything is on and connected");
		return -1;
	}

	printf("Received sample from liberty: timestamp: %f\n",result[0]);
	for (int k=0;k<numSensors;k++) {
		printf("Sensor %d: %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",k,result[k*6+1],result[k*6+2],result[k*6+3],result[k*6+4],result[k*6+5],result[k*6+6]);
	}

	return 0;

}

// SETMODE - set the mode to binary or ASCII
//
// setmode(socket,mode)
// 0 = ASCII
// 1 = binary
void setMode(SOCKET ConnectSocket,int mode) {
	char message[50];
	sprintf(message,"%c,%d",LIBERTY_SetMode,mode);
	printf("Sending message: %s\n",message);
	sendMessage(ConnectSocket,message);
}

// SETHEMISPHERE - set the active hemisphere
// 
// sethemisphere(socket,hemisphere_x,hemisphere_y,hemisphere_z)
//
// e.g. sethemisphere(socket,0,0,0,1) % +Z is active hemisphere for sensor 0
// e.g. sethemisphere(socket,0,0,-1,0); % =Y is the active hemipshere for sensor 0
void sethemisphere(SOCKET ConnectSocket,int sensor,int hemisphere_x,int hemisphere_y,int hemisphere_z) {
	char message[50];
	sprintf(message,"%c,%d,%d,%d,%d",LIBERTY_SetHemisphere,sensor,hemisphere_x,hemisphere_y,hemisphere_z);
	printf("Sending message: %s\n",message);
	sendMessage(ConnectSocket,message);
}

// setsamplerate(socket, samplerate)
//
// samplerate can be 120Hz or 240Hz
void setsamplerate(SOCKET ConnectSocket,int samplerate) {
	int code = -1;
	if (samplerate==120)
		code = 3;
	else if (samplerate==240)
		code = 4;
	else {
		printf("Sample rate must be 120 Hz or 240 Hz - not updated");
		return;
	}

	char message[50];
	sprintf(message,"%c,%d",LIBERTY_SetSampleRate,code);
	printf("Sending message: %s\n",message);
	sendMessage(ConnectSocket,message);
}

// SETUNITS - set the units to cm or inches
//
// setunits(l,units)
// 0 = inches
// 1 = cm
void setunits(SOCKET ConnectSocket,int units) {
	char message[50];
	sprintf(message,"%c,%d",LIBERTY_SetUnits,units);
	printf("Sending message: %s\n",message);
	sendMessage(ConnectSocket,message);
}

// RESETFRAMECOUNT - reset the frame count
void resetframecount(SOCKET ConnectSocket) {
	char message[50];
	sprintf(message,"%c",LIBERTY_ResetFrameCount);
	printf("Sending message: %s\n",message);
	sendMessage(ConnectSocket,message);
}

// SETOUTPUTFORMAT - set the output format
//
// setoutputformat(socket,sensor,recordOrientation)
// recordOrientation should be 0 or 1

void setoutputformat(SOCKET ConnectSocket,int sensor,int recordOrientation) {
	int format;
	if (recordOrientation==1)
		format = 2;
	else
		format = 1;

	char message[50];
	sprintf(message,"%c,%d,%d",LIBERTY_SetOutputFormat,sensor,format);
	printf("Sending message: %s\n",message);
	sendMessage(ConnectSocket,message);
}

// SETALIGNMENTFRAME - set the alignment frame (which way is x,y,z)
//
// setalignmentframe(socket,sensor,coords)
//
// coords should be 9 integers, with the position of the origin (3
// numbers), the direction of the x axis (3 numbers) and the direction of
// the y axis (3 numbers). 
//
// e.g. setalignmentframe(socket,0,0,0,0,0,1,0,0,0,-1)
void setalignmentframe(SOCKET ConnectSocket,int sensor,
	int alignmentframe_0,int alignmentframe_1,int alignmentframe_2,
	int alignmentframe_3,int alignmentframe_4,int alignmentframe_5,
	int alignmentframe_6,int alignmentframe_7,int alignmentframe_8) {

		char message[100];
		sprintf(message,"%c,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",LIBERTY_AlignmentReferenceFrame,sensor,
			alignmentframe_0,alignmentframe_1,alignmentframe_2,
			alignmentframe_3,alignmentframe_4,alignmentframe_5,
			alignmentframe_6,alignmentframe_7,alignmentframe_8);

		printf("Sending message: %s\n",message);
		sendMessage(ConnectSocket,message);
}

// Get a single sample - data size will be numsensors*6+1
void getsinglesample(SOCKET ConnectSocket,int numsensors, double *result) {

	char message[50];
	sprintf(message,"%c,%d",LIBERTY_GetSingleSample,numsensors);
	printf("Sending message: %s\n",message);
	sendMessage(ConnectSocket,message);

	char buffer[1000];
	receiveMessage(ConnectSocket, buffer, 1000);

	// Parse the result
	int i=-1;
	char* remaining = strtok(buffer, ",");
	while (remaining != NULL) {
		i++;
		result[i] = atof(remaining);		
		remaining = strtok(NULL, ",");
	}
}

int getupdaterate(SOCKET ConnectSocket) {

	char message[50];
	sprintf(message,"%c",LIBERTY_GetUpdateRate);
	sendMessage(ConnectSocket,message);

	char buffer[10];
	receiveMessage(ConnectSocket,buffer,10);
	int rawrate = atoi(buffer);

	if (rawrate == 3)
		return 120;
	else if (rawrate == 4)
		return 240;
	else
		return -1;

}