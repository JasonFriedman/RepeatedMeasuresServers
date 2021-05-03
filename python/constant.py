# Message codes for communicating between the client and server

DUMMY = 'D'
GETSAMPLE ='G'
SETUPRECORDING = 'S' # 3 parameters: filename, num markers, maximum time (seconds)
STARTRECORDING = 'T' # no parameters
STOPRECORDING = 'Z' # no parameters
SAVEFILE = 'F' # no parameters
CLOSEDEVICE = 'C' # no parameters
MARKEVENT = 'M' # 1 parameter: number to mark

# Liberty specific
# Set mode to ASCII (0) or binary (1)
LIBERTY_SetMode = '1'
# Set units to inches (0) or cm (1)
LIBERTY_SetUnits = '2'
# Set active hemisphere
LIBERTY_SetHemisphere = '3'
# Set the sample rate (3 = 120Hz, 4 = 240Hz)
LIBERTY_SetSampleRate = '4'
# Reset the frame count
LIBERTY_ResetFrameCount = '5'
#Set output format
LIBERTY_SetOutputFormat = '6'
# Get a single sample
LIBERTY_GetSingleSample = '7'
# Get the update rate
LIBERTY_GetUpdateRate = '8'
# Set the alignment frame (which way is x,y,z)
LIBERTY_AlignmentReferenceFrame = '9'
