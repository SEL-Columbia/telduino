#!/usr/bin/python

#add current timestamp to row 
#create new row with relevant information
#update table

from authorization.clientlogin import ClientLogin
from sql.sqlbuilder import SQL
import ftclient
from fileimport.fileimporter import CSVImporter

uk = "javirosa1912@gmail.com"
pk = "turinghope1848"
token = ClientLogin().authorize(uk, pk)
ft_client = ftclient.ClientLoginFTClient(token)


#show tables
results = ft_client.query(SQL().showTables())
print results

exit()
tableid =1


#for each file in the current directory post W and a timestamp

#read list of files
#get date from file
#delete file

cid = 0
date = ""
#insert row into table
rowid = int(ft_client.query(SQL().insert(tableid, {'date':date, 'ID':cid, 'W':w})).split("\n")[1])
print rowid

