# libertyclient - code for performing basic actions with the Repeated Measures liberty socket server
# e.g. set the sample rate, units, etc.

import constant
import time
import socketclient

# SETMODE - set the mode to binary or ASCII
#
# setMode(socket,mode)
# 0 = ASCII
# 1 = binary
def setmode(s, mode):
    tosend = "%c,%d" % (constant.LIBERTY_SetMode, mode)
    s.send(tosend.encode())

# samplerate can be 120Hz or 240Hz
def setsamplerate(s, samplerate):
    if samplerate == 120:
        code = 3
    elif samplerate == 240:
        code = 4
    else:
        print("Sample rate must be 120 Hz or 240 Hz - not updated")
        return

    tosend = "%c,%d" % (constant.LIBERTY_SetSampleRate, code)
    s.send(tosend.encode())

# SETHEMISPHERE - set the active hemisphere
# sethemisphere(socket, sensor, hemisphere[3])
# e.g. for sensor 0
# sethemisphere(s, 0, [0, -1, 0]); # =Y is the active hemipshere for sensor 0

def sethemisphere(s, sensor, hemisphere):
    tosend = "%c,%d,%d,%d,%d" % (constant.LIBERTY_SetHemisphere, sensor, hemisphere[0], hemisphere[1], hemisphere[2])
    s.send(tosend.encode())

# setunits - set the units to cm (1) or inches (0)
def setunits(s, units):
    tosend = "%c,%d" % (constant.LIBERTY_SetUnits, units)
    s.send(tosend.encode())

# RESETFRAMECOUNT - reset the frame count
def resetframecount(s):
    tosend = "%c" % constant.LIBERTY_ResetFrameCount
    s.send(tosend.encode())

# SETOUTPUTFORMAT - set the output format
# 0 = just position, 1 = also orientation
def setoutputformat(s, sensor, recordorientation):
    if recordorientation == 1:
        format = 2
    else:
        format = 1

    tosend = "%c,%d,%d" % (constant.LIBERTY_SetOutputFormat, sensor, format)
    s.send(tosend.encode())

# SETALIGNMENTFRAME - set the alignment frame (which way is x,y,z)
#
# setalignmentframe(s,sensor,coords)
#
# coords should be 9 integers, with the position of the origin (3
# numbers), the direction of the x axis (3 numbers) and the direction of
# the y axis (3 numbers).
#
# e.g. setalignmentframe(s,0,[0,0,0,0,1,0,0,0,-1])
def setalignmentframe(s, sensor, alignmentframe):
    tosend = "%c,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d" % (constant.LIBERTY_AlignmentReferenceFrame, sensor,
                                                   alignmentframe[0], alignmentframe[1], alignmentframe[2],
                                                   alignmentframe[3], alignmentframe[4], alignmentframe[5],
                                                   alignmentframe[6], alignmentframe[7], alignmentframe[8])

    s.send(tosend.encode())

# Get a single sample - data size will be numsensors*6+1
def getsinglesample(s, numsensors):
    tosend = "%c,%d" % (constant.LIBERTY_GetSingleSample, numsensors)
    s.send(tosend.encode())

    data = s.recv(1000)
    decoded = data.decode("utf-8")
    result = list(map(float,decoded.split(",")))
    return result

def getupdaterate(s):
    tosend = "%c" % constant.LIBERTY_GetUpdateRate
    s.send(tosend.encode())

    data = s.recv(10)
    rawrate = int(data)

    if rawrate == 3:
        return 120
    elif rawrate == 4:
        return 240
    else:
        return -1

# Perform standard setup for the device
# setupdevice(s, nummarkers, samplerate, hemisphere, alignmentframe)
# hemisphere should have 3 elements, e.g. [0 1 0] for the +y hemisphere
# alignment frame should have 9 numbers: [origin(3) x direction (3) y direction (3)]
def setupdevice(s, nummarkers, samplerate, hemisphere, alignmentframe):

    setmode(s, 1)  # Set to binary
    time.sleep(0.5)
    setsamplerate(s, samplerate)  # sample rate to 240 Hz
    time.sleep(0.5)
    # Get update rate to make sure liberty is still answering
    if getupdaterate(s) != samplerate:
        print("Problem after setsamplerate\n")
        return -1

    setunits(s, 1)  # 1 = cm
    time.sleep(0.5)
    # Get update rate to make sure liberty is still answering
    if getupdaterate(s) != samplerate:
        print("Problem after setunits\n")
        return -1

    # set the active hemisphere
    for k in range(nummarkers):
        sethemisphere(s, k, hemisphere)
        time.sleep(0.1)
        if getupdaterate(s) != samplerate:
            print("Problem after sethemisphere\n")
            return -1

    # Reset the frame count
    resetframecount(s)
    time.sleep(0.3)
    if getupdaterate(s) != samplerate:
        print("Problem after resetframecount\n")
        return -1

    # set the output format
    for k in range(nummarkers):
        setoutputformat(s, k, 1)
        time.sleep(0.1)
        if getupdaterate(s) != samplerate:
            print("Problem after setoutputformat\n")
            return -1

    # set the alignment reference frame (which way are origin (3 numbers),
    # x axis(3 numbers), y axis(3 numbers))
    for k in range(nummarkers):
        setalignmentframe(s, k, alignmentframe)
        time.sleep(0.1)
        if getupdaterate(s) != samplerate:
            print("Problem after setalignmentframe\n")
            return -1

    # get a sample to check everything is working
    result = getsinglesample(s, nummarkers)
    if result[0] == 0:
        print("Did not receive a result from the server - check everything is on and connected")
        return -1

    print("Received sample from liberty: timestamp " + str(result[0]) + "\n")
    for k in range(nummarkers):
        todisplay = "Sensor %d: %.3f, %.3f, %.3f, %.3f, %.3f, %.3f" % (k, result[k * 6 + 1], result[k * 6 + 2],
                                                                         result[k * 6 + 3], result[k * 6 + 4],
                                                                         result[k * 6 + 5], result[k * 6 + 6])
        print(todisplay)

    return 0
