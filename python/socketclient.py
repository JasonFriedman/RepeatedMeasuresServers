# Functions for interacting with the Repeated Measures socket servers

import socket
import constant

def setuprecording(s,filename,nummarkers,maxtime):
    tosend = "%c,%s,%d,%d" % (constant.SETUPRECORDING, filename, nummarkers, maxtime)
    s.send(tosend.encode())

def startrecording(s):
    tosend = "%c" % constant.STARTRECORDING
    s.send(tosend.encode())

def stoprecording(s):
    tosend = "%c" % constant.STOPRECORDING
    s.send(tosend.encode())

def savefile(s):
    tosend = "%c" % constant.SAVEFILE
    s.send(tosend.encode())

def closedevice(s):
    tosend = "%c" % constant.CLOSEDEVICE
    s.send(tosend.encode())

def markevent(s,marker):
    tosend = "%c,%d" % (constant.MARKEVENT,marker)
    s.send(tosend.encode())
