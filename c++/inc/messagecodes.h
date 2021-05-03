// Message codes for communicating between the client and server

#define DUMMY 'D'
#define GETSAMPLE 'G'
#define SETUPRECORDING 'S' // 3 parameters: filename, num markers, maximum time (seconds)									
#define STARTRECORDING 'T' // no parameters
#define STOPRECORDING 'Z' // no parameters
#define SAVEFILE 'F' // no parameters
#define CLOSEDEVICE 'C' // no parameters
#define MARKEVENT 'M' // one parameter: A number to mark the event

// Liberty specific
// Set mode to ASCII (0) or binary (1)
#define LIBERTY_SetMode '1'
// Set units to inches (0) or cm (1)
#define LIBERTY_SetUnits '2'
// Set active hemisphere
#define LIBERTY_SetHemisphere '3'
// Set the sample rate (3 = 120Hz, 4 = 240Hz)
#define LIBERTY_SetSampleRate '4'
// Reset the frame count
#define LIBERTY_ResetFrameCount '5'
// Set output format
#define LIBERTY_SetOutputFormat '6'
// Get a single sample
#define LIBERTY_GetSingleSample '7'
// Get the update rate
#define LIBERTY_GetUpdateRate '8'
// Set the alignment frame (which way is x,y,z)
#define LIBERTY_AlignmentReferenceFrame '9'