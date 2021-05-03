#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "SocketClient.h"

int setupdevice(SOCKET ConnectSocket, int numSensors);
void setMode(SOCKET ConnectSocket,int mode);
void sethemisphere(SOCKET ConnectSocket,int sensor,int hemisphere_x,int hemisphere_y,int hemisphere_z);
void setsamplerate(SOCKET ConnectSocket,int samplerate);
int getupdaterate(SOCKET ConnectSocket);
void setunits(SOCKET ConnectSocket,int units);
void resetframecount(SOCKET ConnectSocket);
void setoutputformat(SOCKET ConnectSocket,int sensor,int outputformat);
void setalignmentframe(SOCKET ConnectSocket,int sensor,int alignmentframe_0,int alignmentframe_1,int alignmentframe_2,int alignmentframe_3,int alignmentframe_4,int alignmentframe_5,int alignmentframe_6,int alignmentframe_7,int alignmentframe_8);
void getsinglesample(SOCKET ConnectSocket,int numsensors,double *result);