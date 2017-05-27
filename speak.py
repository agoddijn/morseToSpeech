#!/usr/bin/python2.7
import serial # for serial port
import numpy as np # for arrays, numerical processing
from os import system # for text to speech

#needs: python2, pyserial, numpy,

#define the serial port
port = "/dev/tty.uart-13FF41CE960A3651"

#start our program proper:
#open the serial port
try:
    ser = serial.Serial(port,2400,timeout = 0.050)
    ser.baudrate=9600
# with timeout=0, read returns immediately, even if no data
except:
    print ("Opening serial port",port,"failed")
    print ("Edit program to point to the correct port.")
    print ("Hit enter to exit")
    raw_input()
    quit()

ser.flushInput()

curScentence = ""
curWord = ""
curLetter = ""
readLong = 0
longCount = 0
myLong = 0

while(1): #loop forever
    data = ser.read(1) # look for a character from serial port - will wait for up to 10s (specified above in timeout)
    if len(data) > 0: #was there a byte to read?
        if ord(data) == 0:
            print "End word"
            print curWord
            system('say ' + curWord)
            curScentence += curWord
            curScentence += " "
            curWord = ""
        elif ord(data) == 255:
            print "End Scentence"
            print curScentence
            system('say ' + curScentence)
            curScentence = ""
        else:
            curLetter = chr(ord(data))
            curWord += curLetter
            print curLetter

# DEBUGGING
# while(1): #loop forever
#     data = ser.read(1) # look for a character from serial port - will wait for up to 10s (specified above in timeout)
#     if len(data) > 0: #was there a byte to read?
#         if readLong == 1:
#             myLong = myLong << 8
#             myLong = myLong | ord(data)
#             longCount = longCount + 1
#             if longCount == 4:
#                 readLong = 0
#                 longCount = 0
#                 print myLong
#                 myLong = 0
#         elif ord(data) == 127:
#             print "Reading long Hi:"
#             readLong = 1
#         elif ord(data) == 254:
#             print "Reading long Lo:"
#             readLong = 1
#         elif ord(data) == 0:
#             print "End word"
#             print curWord
#             system('say ' + curWord)
#             curScentence += curWord
#             curScentence += " "
#             curWord = ""
#         elif ord(data) == 255:
#             print "End Scentence"
#             print curScentence
#             system('say ' + curScentence)
#             curScentence = ""
#         else:
#             curLetter = chr(ord(data))
#             curWord += curLetter
#             print curLetter

