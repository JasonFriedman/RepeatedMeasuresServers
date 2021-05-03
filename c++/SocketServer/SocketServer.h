#include <vector>

int listen(char* port);
int sendMessage(char* buffer, int bufferLength);
// These functions should be implemented by the server
double getsample(int nummarkers,double* data);
void idle();
int getdatalength(int nummarkers);
void setupRecording(int nummarkers);
void startRecording();
void stopRecording();
void closeDevice();
void runCommand(char* command,char* parameter1, char* parameter2, char* parameter3,
	char* parameter4, char* parameter5, char* parameter6,
	char* parameter7, char* parameter8, char* parameter9, char* parameter10);
