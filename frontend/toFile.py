#!/usr/bin/python

import serial
import sys
import select
import time

import os
import time
import threading
import re

from authorization.clientlogin import ClientLogin
from sql.sqlbuilder import SQL 
import ftclient

#TODO get table id
#create graph
#Matt's tables as examples in one tab, my tables in another

REPORTREGEX = re.compile("(\d+),\d+,(\d+),\d+,VRMS,(\d+),IRMS,(\d+),W,(\d+),WE,(\d+),(\d+)")
TABLEID =5438389
ser = serial.Serial("/dev/ttyUSB1",9600,timeout=1)

#2853052,2610,0,1,VRMS,30494,IRMS,8112,W,0,WE,0,20000
#ts,seq,#ID,S,V,I,Vp,Ip,per,VA,W,VAE,WE,PF,0,0,StatusCode

class StdinReader(threading.Thread):
    def run(self):
        while True:
            read = sys.stdin.read(1)
            #print "%s"%(read)
            if read:
                ser.write(read)
            

uk = "javirosa1912@gmail.com"
pk = "turinghope1848"
token = ClientLogin().authorize(uk, pk)
ft_client = ftclient.ClientLoginFTClient(token)

#show tables
#results = ft_client.query(SQL().showTables())
#print results
#print "exiting"
#exit()

os.system('stty raw')
stdin_reader = StdinReader()
stdin_reader.start()
while True:
    line = ser.readline()
    m = REPORTREGEX.match(line)
    if m:
        ts,cid,vrms,irms,w,we,stat= map(str.strip,m.groups())
        if int(cid) == 0 or int(cid) == 1:
            print w
            rowF = open(os.path.join("output/",ts),"w")
            date = time.strftime("%Y-%m-%d %H:%M:%S")
            rowF.write(date+","+cid+","+str(w)+"\n")
            rowF.close()
            #insert row into table
            rowid = int(ft_client.query(SQL().insert(TABLEID, {'date':date, 'CID':cid, 'W':w})).split("\n")[1])

            print "cid:%s,w:%s,stat:%s,rowid:%d",(cid,w,stat,rowid)

os.system('stty sane')

