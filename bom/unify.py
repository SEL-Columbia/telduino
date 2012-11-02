import csv
import struct

#FNAME,EAFMT = "TellitShieldV6","4s51s30s90s"
FNAME,EAFMT = "Meter-Daughterboard-input-V6JR","4s50s27s95s"
eacsv,dkcsv = map(lambda f: open(f,'rb'), [FNAME+".ea.csv",FNAME+".dk.csv"])

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
    line[1] = line[1].upper()
    try:
        value,dknum = map(str.strip,line[1].split(':'))
        key = ":".join([device,value])
    except ValueError as ve:
        ##print "No value for %s"%(line)
        notEA.append(line)
        value = ""
        dknum = line[1].strip()
        key = device
    dknum = dknum.split('(')[0].strip().upper() #Take the first number if parenthetical values
    
    #Prefix to the matching line in the dk csv file
    key = key.upper()
    #print "Key \"%s\""%(key)
    try:
        item = dkdict.pop(dknum)
        if len(item) <13:
            item.insert(0,key)
        else:
            dupEA.append(item)
        dkdict[dknum]=item

    except KeyError as ke:
        #print "Not found in dk \"%s\""%(dknum)
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

#TODO fix LEDCHIPLED_1206:GREEN showing up in eanotdk
#CP-002A-ND needs slightly wider holes. 
    
##for line in dk[0:3]:
##    print line
##print
##for line in ea[0:3]:
##    print line
#print ea.next()

#eacsv.close()
#dkcsv.close()
