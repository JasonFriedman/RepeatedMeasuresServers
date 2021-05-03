#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#include "SocketServer.h"
#include "messagecodes.h"

#define DEFAULT_BUFLEN 512

SOCKET ClientSocket = INVALID_SOCKET;

// Based on http://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
int listen(char* port) {
	WSADATA wsaData;
	int iResult;

	int currentSample = 0; // >1 means sampling, 0 means waiting
	int recordedSamples = 0;
	int maximumframes = 0; // To be set by the client
	int nummarkers = 0; // To be set by the client
	double* data;
	double** databuffer;
	double* sampletime;
	double framenumber;
	double lastframe = 0;
	int marker=0;
	int datalength;
	SYSTEMTIME st;
	char filename[100];
	int framerate = 240;

	SOCKET ListenSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, port, &hints, &result);
	if ( iResult != 0 ) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("Listening on port %s\n",port);

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("Accepted a client\n");

	// No longer need server socket
	closesocket(ListenSocket);

	// Send an acknowledgement (R)
	send(ClientSocket, "R\n", 2, 0); 

	printf("Sent acknowlegement (R)\n");

	// Set a timeout
	DWORD timeout = 1; // ms
	setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	
	// Receive until the peer shuts down the connection
	do {
		if (currentSample>0) {
			if (currentSample > maximumframes) {
				printf("Buffer overflow (%d > %d)! - Not enough memory allocated - check that the sample rate is sufficient and the program is keeping up!!!!!!! Frames will be lost\n",
					currentSample,maximumframes);
			} else {
				double *tmpdata;
				tmpdata = (double*) malloc(datalength*sizeof(double));				 
				framenumber = getsample(nummarkers,tmpdata);
				//printf("Framenumber is %.3f, lastframe is %.3f\n",framenumber,lastframe);

				if ((framenumber-lastframe) > 0.0001 || marker>0) {
					GetSystemTime(&st);
					sampletime[currentSample-1] = st.wHour * 60 * 60 * 1000 + st.wMinute * 60 * 1000 + st.wSecond*1000 + st.wMilliseconds;
					for (int k=0;k<datalength;k++)
						databuffer[k][currentSample-1] = tmpdata[k];
					databuffer[datalength][currentSample-1] = marker;
					marker=0;
					lastframe = framenumber;
					// increment sample number if recording
					if (maximumframes != -1) {
						currentSample = currentSample + 1;
					}
				}
				free(tmpdata);

			}
		} else {
			idle();
		}

		clock_t begin = clock();
		// Check whether there is anything to read
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(ClientSocket, &read_fds);
		struct timeval tv = {0,100};

		int selectresult = select(0,&read_fds,0,0,&tv);
		//printf("Select result is %d\n",selectresult);

		if (selectresult>0)
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		else
			iResult = SOCKET_ERROR;

		clock_t end = clock();

		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		//printf("Elasped time:%.4f\n",elapsed_secs);


		if (iResult == SOCKET_ERROR)
		{
			if (selectresult<=0 || WSAGetLastError() == WSAETIMEDOUT) {
				iResult = 10;
			} else {
				printf("Error in receiving with the socket: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
		} else if (iResult == 0) {
			printf("Connection closing...\n");
		} else {
			recvbuf[iResult] = '\0';
			//printf("Message size %d received: %s\n", iResult, recvbuf);

			char* command;
			char* parameter1;
			char* parameter2;
			char* parameter3;
			char* parameter4;
			char* parameter5;
			char* parameter6;
			char* parameter7;
			char* parameter8;
			char* parameter9;
			char* parameter10;

			command = strtok(recvbuf, ",");
			parameter1 = strtok(NULL,",");
			if (parameter1 != NULL) {
				parameter2 = strtok(NULL,",");
				if (parameter2 != NULL) {
					parameter3 = strtok(NULL,",");
					if (parameter3 != NULL) {
						parameter4 = strtok(NULL,",");
						if (parameter4 != NULL) {
							parameter5 = strtok(NULL,",");
							if (parameter5 != NULL) {
								parameter6 = strtok(NULL,",");
								if (parameter6 != NULL) {
									parameter7 = strtok(NULL,",");
									if (parameter7 != NULL) {
										parameter8 = strtok(NULL,",");
										if (parameter8 != NULL) {
											parameter9 = strtok(NULL,",");
											if (parameter9 != NULL) {
												parameter10 = strtok(NULL,",");
											}
										}
									}
								}
							}
						}
					}
				}
			}

			switch(recvbuf[0]) {
			case DUMMY:
				break;
			case GETSAMPLE:
				{nummarkers = std::stoi(parameter1);
				char *outputbuffer;
				outputbuffer = (char*) malloc(8*(datalength+1));
				int which = 0;
				if (currentSample>1)
					which = currentSample-1;
				else if (currentSample==1)
					which = 0;
				for (int k=0;k<datalength+1;k++) {
					sprintf(&outputbuffer[k*8],"%3.4f",databuffer[k][which]);
				}
				send(ClientSocket, outputbuffer, 8*(datalength+1), 0);
				free(outputbuffer);}
				break;

			case SETUPRECORDING:
				{
					strcpy(filename,parameter1);
					nummarkers = std::atoi(parameter2);
					int maximumtime = std::atoi(parameter3);
					maximumframes = maximumtime * framerate;
					printf("Setting up recording with filename %s, %d markers, max time %d seconds, maximum frames %d\n",filename,nummarkers,maximumtime,maximumframes);
					datalength = getdatalength(nummarkers); // number of numbers
					databuffer=(double **) malloc((datalength+1)*sizeof(char *));
					for (int k=0;k<datalength+1;k++) {
						databuffer[k] = (double*) malloc(maximumframes*sizeof(double));
					}
					sampletime = (double*) malloc(maximumframes*sizeof(double));
					setupRecording(nummarkers);
				}
				break;

			case STARTRECORDING:
				printf("Starting recording\n");
				currentSample = 1;
				startRecording(); 
				break;

			case STOPRECORDING:
				printf("Stopping recording\n");
				stopRecording();
				recordedSamples = currentSample-1;
				currentSample = 0;
				break;

			case SAVEFILE:
				{ printf("Saving %d samples with datalength %d to file %s\n",recordedSamples,datalength,filename);
				FILE *fp = fopen(filename,"w");
				for(int line=0;line<recordedSamples;line++) {
					for (int d=0;d<=datalength;d++) {
						if (d>0) {
							fprintf(fp,",%.8f",databuffer[d][line]);
						} else {
							fprintf(fp,"%.8f",databuffer[d][line]);
						}
					}
					fprintf(fp,"\n");
				}
				fclose(fp);
				free(databuffer);
				free(sampletime);
				}
				break;

			case MARKEVENT:
				{ int eventNumber = atoi(parameter1);
				  printf("Marking event %d\n", eventNumber);
				  marker = eventNumber;
				}
				break;

			case CLOSEDEVICE:
				printf("Closing device\n");
				iResult = 0;
				break;

			default: // Leave it to the device to deal with it
				runCommand(command,parameter1,parameter2,parameter3,parameter4,parameter5,parameter6,parameter7,parameter8,parameter9,parameter10);
			}
		}

		// Echo the buffer back to the sender
		/*     iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
		if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
		}
		printf("Bytes sent: %d\n", iSendResult);
		}
		*/

		} while (iResult > 0);

		// shutdown the connection since we're done
		printf("Shutting down the connection\n");
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		// cleanup
		closesocket(ClientSocket);
		WSACleanup();

		return 0;
	}

	int sendMessage(char* buffer, int bufferLength) {
		int iSendResult = send( ClientSocket, buffer, bufferLength, 0 );
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return -1;
		}
		//printf("Bytes sent: %d\n", iSendResult);
		return iSendResult;
	}