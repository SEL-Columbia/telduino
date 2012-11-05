import csv
import struct

COMPLETELINESIZE = 15
NOTES = 13
EAKEY = 14
EAGLEDELIMETER = "\t"

FNAME,EAFMT = "TellitShieldV6","4s51s30s90s"
#FNAME,EAFMT = "Meter-Daughterboard-input-V6JR","4s50s27s95s"
eacsv,dkcsv = map(lambda f: open(f,'rb'), [FNAME+".ea.csv",FNAME+".dk.csv"])
outfile = open(FNAME + ".csv",'w')
#CLEAR EA
eacsv.readline()
eacsv.readline()
eacsv.readline()

dkcsv.readline()

ea = [map(str.strip,struct.unpack(EAFMT,line)) for line in eacsv]
#print struct.calcsize(EAFMT)

#ea = csv.reader(eacsv,dialect=csv.excel_tab)
dk = list(csv.reader(dkcsv,dialect=csv.excel))
dkdict = {}
for item in dk:
    item.append("") #NOTES
    item.append("") #ea Key
    dkdict[item[0].strip().upper()]=item
#print dkdict.keys()

notDK = []
notEA = []
dupEA = []
inEAnotDK = []
#For each key in the ea file
for line in ea:
    #Generate key and get dk part number
    device = line[2].strip()
    try:
        value,dknum = map(str.strip,line[1].split(':'))
        key = ":".join([device,value])
    except ValueError as ve:
        ##print "No value for %s"%(line)
        notEA.append(line)
        value = ""
        dknum = line[1].strip()
        key = device + ":"
    dknum = dknum.split('(')[0].strip().upper() #Take the first number if parenthetical values
    oldNumbers = "/".join(dknum.split('(')[1:]) +" "
    #Prefix to the matching line in the dk csv file
    keyLower = key
    key = key.upper()
    #print "Key \"%s\""%(key)
    try:
        item = dkdict.pop(dknum)
        if len(item) < COMPLETELINESIZE:
            item.insert(0,key)
            item[EAKEY] = keyLower
        else:
            dupEA.append(item)
        item[NOTES] = item[NOTES] + oldNumbers
        dkdict[dknum]=item
    except KeyError as ke:
        notDK.append((dknum,key))

print "No value for part in EA. n=%d"%(len(notEA))
for i in notEA:
    print i
print
print "Not found in DK BOM. n=%d"%(len(notDK))
for i in notDK:
    print i
print
print "Duplicate part lines found in EA. n=%d"%(len(dupEA))
for i in dupEA:
    print i[0]

for dkPart in dkdict.keys():
    found = False
    for line in ea:
        if line[1].find(dkPart) != -1:
            found=True
    if not found:
        inEAnotDK.append(dkdict[dkPart])
print
print "Extra parts in DK BOM not found in EA. n=%d"%(len(inEAnotDK))
for i in inEAnotDK:
    print i[0:3]

#GENERATE DATABASE FOR EAGLE
outfile.write(EAGLEDELIMETER.join("Key,PartNumber,Distributor,Manufacturer,Notes\n".split(","))
for key,item in dkdict.iteritems():
    if len(item) == COMPLETELINESIZE:
        line = EAGLEDELIMETER.join([item[EAKEY],key,"Digikey",item[2],item[NOTES] +"\n"])
        outfile.write(line)

#CLEAN UP EAGLE FILES

#CP-002A-ND needs slightly wider holes. 
    

eacsv.close()
dkcsv.close()
outfile.close()
