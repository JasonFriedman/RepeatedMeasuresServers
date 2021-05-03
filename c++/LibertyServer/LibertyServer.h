#define _WIN32_LEAN_AND_MEAN
#include <windows.h>

bool InitializeSystem();
bool ConnectToSystem();
bool SetupDevice();
void Disconnect();
int readSingleSample(double currentData[]);
bool readSampleContinuous(double currentData[],int numMarkers, int dataRequested);
bool StartContinuousSampling();
bool StopContinuousSampling();
int GetFrameRate();
bool SetBinary(bool useBinary);
bool SetMetric(bool useMetric);
bool SetHemisphere(int station,float x,float y,float z);
bool SetFrameRate(int frameRate);
bool ResetFrameCount();
bool SetOutputFormat(int station,int outputType);
bool ClearAlignmentFrame(int station);
bool SetAlignmentFrame(int station,float O1,float O2,float O3,float X1,float X2,float X3,float Y1,float Y2,float Y3);
void LibertyServer(unsigned int cmd, int param0 = 0, float param1 = 0, float param2 = 0, float param3 = 0, float param4 = 0, float param5 = 0, float param6 = 0, float param7 = 0, float param8 = 0, float param9 = 0, double* output = NULL);

double getsample(int nummarkers,double* data);
void idle();
int getdatalength(int nummarkers);
void setupRecording(int nummarkers);
void startRecording();
void stopRecording();