#include "LibertyClient.h"

// This is a trivial example of a liberty client

int __cdecl main(int argc, char **argv) 
{
	// Validate the parameters
	if (argc != 3) {
		printf("usage: %s server-name port\n", argv[0]);
		return 1;
	}

	SOCKET ConnectSocket = connectToSocket(argv[1],argv[2]);

	if (ConnectSocket==1) {
		printf("Failed to connect, quitting\n");
		return 1;
	}

	int nummarkers = 1;

	setupdevice(ConnectSocket,nummarkers);

	Sleep(2000);

	char message[50];
	char filename[] = "testfile.csv";

	int maxtime = 11;

    setupRecording(ConnectSocket, filename,nummarkers,maxtime);
	Sleep(2000);
    
    startRecording(ConnectSocket); // record for 10 seconds, mark event every two seconds
	Sleep(2000); 
	markEvent(ConnectSocket,2);
	Sleep(2000);
	markEvent(ConnectSocket,2);
	Sleep(2000);
	markEvent(ConnectSocket,2);
	Sleep(2000);
	markEvent(ConnectSocket,2);
	Sleep(2000);
	
    stopRecording(ConnectSocket);
	Sleep(2000);

    saveFile(ConnectSocket);
	Sleep(2000);
    
    closeDevice(ConnectSocket);
	Sleep(2000);

	closeSocket(ConnectSocket);
}