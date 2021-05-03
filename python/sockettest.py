#!/usr/bin/env python3

import socket
import socketclient
import libertyclient
import time

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 9998  # The port used by the server
nummarkers = 1  # number of liberty markers
samplerate = 240 # Hz
hemisphere = [0,1,0] # +y hemisphere
alignment_reference_frame = [0, 0, 0, 1, 0, 0, 0, 1, 0] # origin (3), x direction (3), y direction (3)
filename = 'pythontest.csv'
maxtime = 10 # seconds

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    data = s.recv(1024)
    decoded = data.decode("utf-8")

    if decoded[0] != 'R':
        print('Did not receive acknowledgement, rather received ' + decoded[0])
        exit(0)
    else:
        print('Received acknowledgement')

    libertyclient.setupdevice(s, nummarkers, samplerate, hemisphere, alignment_reference_frame)
    time.sleep(2.0)

    socketclient.setuprecording(s,filename,nummarkers,maxtime+1)
    time.sleep(2.0)

    socketclient.startrecording(s)

    time.sleep(maxtime/5)
    socketclient.markevent(s,2)

    time.sleep(maxtime/5)
    socketclient.markevent(s,2)

    time.sleep(maxtime/5)
    socketclient.markevent(s,2)

    time.sleep(maxtime/5)
    socketclient.markevent(s,2)

    time.sleep(maxtime/5)
    socketclient.stoprecording(s)
    
    time.sleep(2.0)

    socketclient.savefile(s)
    time.sleep(2.0)

    socketclient.closedevice(s)
