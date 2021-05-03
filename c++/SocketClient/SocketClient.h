#ifndef socketclient_h
#define socketclient_h

SOCKET connectToSocket(char* host,char* port);
int sendMessage(SOCKET ConnectSocket, char* sendbuf);
int receiveMessage(SOCKET ConnectSocket, char* recvbuf, int recvbuflen);
int closeSocket(SOCKET ConnectSocket);

void setupRecording(SOCKET ConnectSocket, char* filename, int nummarkers, int maxtime);
void startRecording(SOCKET ConnectSocket);
void stopRecording(SOCKET ConnectSocket);
void saveFile(SOCKET ConnectSocket);
void closeDevice(SOCKET ConnectSocket);
void markEvent(SOCKET ConnectSocket, int eventNumber);

#endif /* socketclient_h */
