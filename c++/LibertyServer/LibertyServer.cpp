#include "LibertyServer.h"

#include <string>
#include <iostream>
#include "tchar.h"
#include "SocketServer.h"

#include "PDI.h"

#include "messagecodes.h"

using namespace std ;

CPDIdev		g_pdiDev;
CPDImdat    g_pdiMDat;
CPDIser		g_pdiSer;
DWORD		g_dwFrameSize;
BOOL		g_bCnxReady;
DWORD		g_dwStationMap;

#define BUFFER_SIZE 0x1FA400   // 30 seconds of xyzaer+fc 8 sensors at 240 hz
//BYTE	g_pMotionBuf[0x0800];  // 2K worth of data.  == 73 frames of XYZAER
BYTE	g_pMotionBuf[BUFFER_SIZE];

void LibertyServer(unsigned int cmd, int param0, float param1, float param2, float param3, float param4, float param5, float param6, float param7, float param8, float param9, double* output)
{

	double *outArray;
	double *currentData;
	int numMarkers;
	int maxMarker; // Highest marker number connected (1 = marker 1, etc.)
	
	switch(cmd) {
		// Initialize the system
	case 0:
		{InitializeSystem();
		bool status = ConnectToSystem();
		if (!status)
			exit(-1);
		SetupDevice();}
		break;

		// Get a single sample (when not in continuous mode)
	case 1:
		{if(param0==0) {
			printf("ERROR: The second argument must specify the highest marker number connected.\n");
			return;
		}
		maxMarker = param0;
		// sample size = num markers * 6 + 1
		int outputLength = readSingleSample(output);
		if (outputLength != maxMarker*6 + 1)
			printf("Incorrect number of markers specified, outputLength is %d and not %d\n",outputLength,maxMarker * 6 + 1);
		}
		break;

		// Start continuous mode
	case 2:
		StartContinuousSampling();
		break;

		// Get a sample in continuous mode
	case 3: {
		if(param0==0 || param1==0) {
			printf("The second argument must specify the maximum marker and the third the data requested.\n");
			return;
		}
		maxMarker = param0;
		int dataRequested = param1; // 1=just position, 2 = position + orientation

		// sample size = num markers * 6 + 1
		bool success = readSampleContinuous(output,maxMarker,dataRequested);
		
		if (!success) {
			for (int k=0;k<maxMarker*dataRequested*3+1;k++)
				output[k] = 0;
		}
		//else {
		//	for (int k=0;k<numMarkers*dataRequested*3+1;k++) {
		//		printf("%.3f\n",output[k]);
		//}

		break;
		}

		// Stop sampling in continuous mode
	case 4:
		StopContinuousSampling();
		break;

		// Disconnect
	case 5:
		Disconnect();
		break;

		// Get the frame rate
	case 6:
		output[0] = GetFrameRate();		
		break;

		// Set binary / ASCII
	case 7:
		{
			if(param0==0) {
				printf("ERROR: The second argument must specify whether to use binary (1) or ASCII (0).\n");
				return;
			}
			bool useASCII = (param0>0?true:false);
			SetBinary(useASCII);
			break;
		}

		// Set units (0=inches, 1 = cm)
	case 8:
		{			
			bool units = (param0>0?true:false);
			SetMetric(units);
			break;
		}


		// Set hemisphere (station, x, y, z) xyz is a unit vector in the direction of the hemisphere
	case 9:
		{
			int station = param0;
			float x = param1;
			float y = param2;
			float z = param3;

			SetHemisphere(station,x,y,z);
			break;
		}
		// Set the frame rate
	case 10:
		{
			if(param0==0) {
				printf("ERROR: The second argument must be the frame rate (60 or 120 or 240).\n");
				return;
			}
			int frameRate = param0;
			SetFrameRate(frameRate);
			break;
		}

		// Reset the frame counter
	case 11:
		ResetFrameCount();
		break;

		// Set the output type
	case 12:
		{
			int station = param0;
			int outputType = (int) param1;
			SetOutputFormat(station,outputType);
			break;
		}

	case 13:
		{
			int station = (int) param0;
			ClearAlignmentFrame(station);
			break;
		}
	case 14:
		{
			int station = param0;
			float O1 = param1;
			float O2 = param2;
			float O3 = param3;
			float X1 = param4;
			float X2 = param5;
			float X3 = param6;
			float Y1 = param7;
			float Y2 = param8;
			float Y3 = param9;

			SetAlignmentFrame(station,O1,O2,O3,X1,X2,X3,Y1,Y2,Y3);
			break;
		}
	}

}

bool InitializeSystem()
{
	bool retVal = true;

	g_pdiDev.Trace(true, 7);

	g_pdiMDat.Empty();
	g_pdiMDat.Append( PDI_MODATA_POS );
	g_pdiMDat.Append( PDI_MODATA_ORI );
	g_pdiMDat.Append( PDI_MODATA_FRAMECOUNT );
	g_pdiMDat.Append( PDI_MODATA_CRLF );
	g_dwFrameSize = 8+12+12+4+2;

	g_bCnxReady = FALSE;
	g_dwStationMap = 0;

	return retVal;
}

bool ConnectToSystem( )
{
	if (!(g_pdiDev.CnxReady()))
	{
		g_pdiDev.SetSerialIF( &g_pdiSer );

		ePiCommType eType = g_pdiDev.DiscoverCnx();
		switch (eType)
		{
		case PI_CNX_USB:
			cout << "USB Connection: ";
			break;
		case PI_CNX_SERIAL:
			cout << "Serial Connection: ";
			break;
		default:
			cout << "DiscoverCnx result: ";
			break;
		}
		cout << g_pdiDev.GetLastResultStr() << endl;
		g_bCnxReady = g_pdiDev.CnxReady();

	}
	else
	{
		cout << "Already connected" << endl;
		g_bCnxReady = true;
	}

	if (g_bCnxReady==0) {
		printf("Can't connect to Liberty. Is it turned on? It may need to be reset\n");
		return false;
	}

	return (g_bCnxReady>0);
}

bool SetupDevice()
{
	g_pdiDev.SetPnoBuffer( g_pMotionBuf, BUFFER_SIZE );
	cout << "SetPnoBuffer" << endl;

	g_pdiDev.StartPipeExport();
	cout << "StartPipeExport" << endl;

	g_pdiDev.SetSDataList( -1, g_pdiMDat );
	cout << "SetSDataList" << endl;

	CPDIbiterr cBE;
	g_pdiDev.GetBITErrs( cBE );
	cout << "GetBITErrs" << endl;

	if (!(cBE.IsClear()))
	{
		g_pdiDev.ClearBITErrs();
		cout << "ClearBITErrs" << endl;
	}

	return true;
}


int readSingleSample(double currentData[]) {

	PBYTE pBuf;
	DWORD dwSize;
	int sampleSize = 6;

	if (!(g_pdiDev.ReadSinglePnoBuf(pBuf, dwSize)))
	{
		printf("Error in reading a frame\n");
		return -1;
	}
	DWORD i =0;

	int lastSensor;

	while (i<dwSize)
	{
		BYTE sensorNum = pBuf[i+2];
		SHORT shSize = pBuf[i+6];

		// skip rest of header
		i += 8;

		PFLOAT pPno = (PFLOAT)(&pBuf[i]);
		for (int k=0;k<sampleSize;k++) {
			currentData[(sensorNum-1)*sampleSize+k+1] = pPno[k];
		}
		PDWORD pFrameCount  = (PDWORD)(&pBuf[i+sampleSize*4]);
		currentData[0] =  (float) *pFrameCount;

		lastSensor = (int)sensorNum;

		i += shSize;
	}
	return lastSensor*sampleSize+1;
}

bool readSampleContinuous(double currentData[],int maxMarker,int dataRequested) {

	PBYTE pBuf;
	DWORD dwSize;
	int sampleSize;
	int lastSensor;

	if (dataRequested==1)
		sampleSize = 3;
	else
		sampleSize = 6;

	if (!(g_pdiDev.LastPnoPtr(pBuf, dwSize)))
	{
		printf("Error in reading a frame during continuous recording\n");
		return false;
	}

	// no data ready
	if (dwSize==0) {
		printf("No data available\n");
		return false;
	}

	// per sensor = 8(header) + 3 * 4 + 4 (timestamp) + 2 (CR/LF) = 16  [just position]
	//              8(header) + 6 * 4 + 4 (timestamp) + 2 (CR/LF) = 38  [position + orientation]

	// If not all markers are connected, the size will be smaller than expected, so don't check
	//int expectedSize = numMarkers * (8 + sampleSize * 4 + 4 + 2);
	// no data ready
	//if (dwSize!=expectedSize) {
	//	printf("Data is not the expected size (%d when should be %d)\n",dwSize,expectedSize);
	//	return false;
	//} 

	DWORD i =0;

	while (i<dwSize)
	{
		BYTE sensorNum = pBuf[i+2];
		SHORT shSize = pBuf[i+6];

		//printf("SensorNum is %d\n",sensorNum);

		// skip rest of header
		i += 8;

		PFLOAT pPno = (PFLOAT)(&pBuf[i]);
		for (int k=0;k<sampleSize;k++) {
			currentData[(sensorNum-1)*sampleSize+k+1] = pPno[k];
		}
		// Read the frame number
		PDWORD pFrameCount  = (PDWORD)(&pBuf[i+sampleSize*4]);
		currentData[0] =  (float) *pFrameCount;
		
		lastSensor = (int)sensorNum;

		i += shSize;
	}
	if (maxMarker != lastSensor) {
		printf("Last sensor number %d does not match the predicted %d\n",lastSensor,maxMarker);
		return false;
	}

	return true;
}

void Disconnect()
{
	if (!(g_pdiDev.CnxReady()))
	{
		cout << "Already disconnected\n";
	}
	else
	{
		g_pdiDev.Disconnect();
		cout << "Disconnect result: " + string(g_pdiDev.GetLastResultStr()) + "\r\n";
	}
}

bool StartContinuousSampling() {

	if (!(g_pdiDev.StartContPno(0))) {
		printf("Error in starting continuous sampling\n");
		return false;
	}
	return true;

}

bool StopContinuousSampling() {

	if (!(g_pdiDev.StopContPno())) {
		printf("Error in stopping continuous sampling\n");
		return false;
	}
	return true;

}

int GetFrameRate() {
	ePiFrameRate eRate;

	g_pdiDev.GetFrameRate(eRate);

	if (eRate==PI_FRATE_60) {
		printf("Frame rate is 60 Hz\n");
		return 60;
	} else if(eRate==PI_FRATE_120) {
		printf("Frame rate is 120 Hz\n");
		return 120;		
	} else if(eRate==PI_FRATE_240) {
		printf("Frame rate is 120 Hz\n");
		return 240;		
	} else {
		printf("Error getting frame rate\n");
		return -1;		
	}
}

bool SetBinary(bool useBinary) {
	return (g_pdiDev.SetBinary(useBinary)>0);
}

bool SetMetric(bool useBinary) {
	return (g_pdiDev.SetMetric(useBinary)>0);
}

bool SetHemisphere(int station,float x,float y,float z) {
	PDI3vec hemisphereVec = {x,y,z};

	return (g_pdiDev.SetSHemisphere ( station, hemisphereVec)>0);
}

bool SetFrameRate(int frameRate) {
	ePiFrameRate eRate;

	if (frameRate==60)
		eRate=PI_FRATE_60;
	else if(frameRate==120)
		eRate=PI_FRATE_120;
	else if(frameRate==240)
		eRate=PI_FRATE_240;
	else
		printf("Error in setting frame rate: unknown value %d\n",frameRate);

	return (g_pdiDev.SetFrameRate(eRate)>0);
}

bool ResetFrameCount() {
	return (g_pdiDev.ResetFrameCount()>0);
}

// outputType = 1 -> just position (+ framenumber), = 2 -> position and orientation (+framenumber)
bool SetOutputFormat(int station,int outputType) {
	CPDImdat m_pdiMDat;
	m_pdiMDat.Empty(); // binary header = 8
	m_pdiMDat.Append( PDI_MODATA_POS ); // size 12
	if (outputType==2) {
		m_pdiMDat.Append( PDI_MODATA_ORI ); // size 12
	}
	m_pdiMDat.Append( PDI_MODATA_FRAMECOUNT); // size 4
	m_pdiMDat.Append( PDI_MODATA_CRLF ); // size 2

	printf("updating output format for station %d\n",station);
	return(g_pdiDev.SetSDataList (station, m_pdiMDat)>0);
}

bool ClearAlignmentFrame(int station) {
	return(g_pdiDev.ResetSAlignment (station)>0);
}

bool SetAlignmentFrame(int station,float O1,float O2,float O3,float X1,float X2,float X3,float Y1,float Y2,float Y3) {
	PDI3vec O = {O1,O2,O3};
	PDI3vec X = {X1,X2,X3};
	PDI3vec Y = {Y1,Y2,Y3};

	printf("updating alignment frame for station %d\n",station);
	return(g_pdiDev.SetSAlignment(station,O,X,Y)>0);
}

double getsample(int maxMarker,double* data) {
	// For now, fixed to position + orientation
	LibertyServer((unsigned int)3,maxMarker,(float)2,(float)0,(float)0,(float)0,(float)0,(float)0,(float)0,(float)0,(float)0,data);
	double frame =data[0];
	return frame;
}

void idle() {

}

int getdatalength(int nummarkers) {
	return nummarkers * 6+1;
}

void setupRecording(int nummarkers) {

}

void startRecording() {
	LibertyServer(2);
}

void stopRecording() {
	LibertyServer(4);
}

void closeDevice() {

}

void runCommand(char* command,char* parameter1, char* parameter2, char* parameter3,
	char* parameter4, char* parameter5, char* parameter6,
	char* parameter7, char* parameter8, char* parameter9, char* parameter10) {

		char commandname = *command;
		
		switch (commandname) {
		case LIBERTY_SetMode:
			LibertyServer(7,atoi(parameter1));
			printf("Set mode to %d [0=ascii, 1=binary]\n",atoi(parameter1));
			break;
		case LIBERTY_SetUnits:
            // set units to inches (0) or cm (1)
            LibertyServer(8,atoi(parameter1));
			printf("Set units to %d [0=inches, 1=cm]\n",atoi(parameter1));
			break;
		case LIBERTY_SetHemisphere:
            // Set the hemisphere
			{int sensor = atoi(parameter1);
			double hemisphere_x = (double) atoi(parameter2);
			double hemisphere_y = (double) atoi(parameter3);
			double hemisphere_z = (double) atoi(parameter4);
			LibertyServer(9,sensor,hemisphere_x,hemisphere_y,hemisphere_z);
			printf("Set hemisphere for sensor %d to %.1f,%.1f,%.1f\n",sensor,hemisphere_x,hemisphere_y,hemisphere_z);}
            break;
		case LIBERTY_SetSampleRate:
            // set sample rate (3=120Hz, 4=240Hz)
			{int framerate = 0;
			 int selection = atoi(parameter1);
			 if (selection==2)
				 framerate = 60;
			 else if (selection==3)
				 framerate = 120;
			 else if (selection==4)
				 framerate = 240;
			 else
				 printf("Unknown frame rate: %d\n",selection);
			 LibertyServer(10,framerate);
			 printf("Set the framerate to %d Hz\n",framerate);}
            break;
		case LIBERTY_ResetFrameCount:
            // Reset the frame count
            LibertyServer(11);
            break;
		case LIBERTY_SetOutputFormat:
            LibertyServer(12,atoi(parameter1),(double)atoi(parameter2));
            break;
		case LIBERTY_GetSingleSample:
			{int maxMarker = atoi(parameter1);
				
			double output[100];
			LibertyServer(1,maxMarker,0,0,0,0,0,0,0,0,0,output);

			int datalength = maxMarker*6 + 1;
			char buffer[1000];
			int i=0;
			for (int k=0;k<datalength;k++) {
				// 15 characters per number
				sprintf(&buffer[i],"%015.4f",output[k]);
				i+=15;
				if (k<datalength-1) {
					sprintf(&buffer[i],",");
					i++;
				}				
			}

			// Return the results
			sendMessage(buffer,i);
			printf("Sent the message with length %d: %s\n",i,buffer);
			}
            break;
		case LIBERTY_GetUpdateRate:			
			double output;
			LibertyServer(6,0,0,0,0,0,0,0,0,0,0,&output);
            
			int returnValue;
            if (output==60)
                returnValue = 2;
            else if (output==120)
                returnValue = 3;
            else if (output==240)
                returnValue = 4;
            else
                printf("Error: unknown update rate %f\n",output);

			//printf("update rate is %d\n",returnValue);
			char outputString[2];
			sprintf(outputString,"%d\n",returnValue);
			sendMessage(outputString,2);
            break;
		case LIBERTY_AlignmentReferenceFrame:
            // clear the previous reference frame
            { int sensor = atoi(parameter1);
			  double zero1 = (double) atoi(parameter2);
			  double zero2 = (double) atoi(parameter3);
			  double zero3 = (double) atoi(parameter4);
			  double x1 = (double) atoi(parameter5);
			  double x2 = (double) atoi(parameter6);
			  double x3 = (double) atoi(parameter7);
			  double y1 = (double) atoi(parameter8);
			  double y2 = (double) atoi(parameter9);
			  double y3 = (double) atoi(parameter10);
				
			  LibertyServer(13,sensor);
              LibertyServer(14,sensor,zero1,zero2,zero3,
				  x1,x2,x3,y1,y2,y3);}				
            break;
		default:
			printf("Unknown command %s\n",command);
		}
}