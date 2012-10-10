#!/usr/bin/python

import serial
import sys
import select
import time

import os
import time
import threading
import re

REPORTREGEX = re.compile("(\d+),\d+,(\d+),\d+,VRMS,(\d+),IRMS,(\d+),W,(\d+),WE,(\d+),(\d+)")
#2853052,2610,0,1,VRMS,30494,IRMS,8112,W,0,WE,0,20000
#ts,seq,#ID,S,V,I,Vp,Ip,per,VA,W,VAE,WE,PF,0,0,StatusCode

ser = serial.Serial("/dev/ttyUSB0",9600,timeout=1)

class StdinReader(threading.Thread):
    def run(self):
        while True:
            read = sys.stdin.read(1)
            #print "%s"%(read)
            if read:
                ser.write(read)
            

os.system('stty raw')
stdin_reader = StdinReader()
stdin_reader.start()

while True:
    line = ser.readline()
    m = REPORTREGEX.match(line)
    if m:
        ts,id,vrms,irms,w,we,stat= m.groups()
        if int(id.strip()) == 0 or int(id.strip()) == 1:
            rowF = open(os.path.join("output/",str(ts)),"w")
            rowF.write(time.strftime("%Y-%m-%d %H:%M:%S")+","+str(id)+","+str(w))
            rowF.close()
            print int(id)

os.system('stty sane')
    
exit()
    
