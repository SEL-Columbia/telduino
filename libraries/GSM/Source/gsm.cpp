#include "gsmbase.h"

//with debug
GSMbase::GSMbase(Serial& _telit ,
uint32_t(*_millis)(),Serial* _debug) 
:telitPort(_telit),millis(_millis),DebugPort(_debug)
,fullData(NULL),parsedData(NULL)
{}


//////////////////////////////////////////////////////////////////////SEND FUNCS
//Sends the command clear (use with catchTelitData)
void GSMbase::sendATCommand(const char* theMessageMelleMel){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
	
}
//Sends AT command and checks for OK or ERROR good for commands that reply OK or ERROR
const char* const GSMbase::sendRecQuickATCommand(const char* theMessageMelleMel){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return	(catchTelitData(2000,true));				// 2000 milli time out with a quickCheck return data

}
//Sends AT command and stores reply with the catch data function
const char* const GSMbase::sendRecATCommand(const char* theMessageMelleMel){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return	(catchTelitData());								//return data

}
//Sends AT command parses reply, makes string from the passed in strings
//EG. 
//input string: "the quick brown blah blah"
//_start string: "the"
//_end string: "blah"
//returns a pointer to "quick brown"
const char* const GSMbase::sendRecATCommandParse(const char* theMessageMelleMel, 
const char* _start,const char* _end){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return (parseData(const_cast<char*>(catchTelitData()),_start,_end));			//return data

}
//Sends AT command splits data according to delimeter
//EG
//input string: "the, quick: brown, blah: blah,"
//_delimiters string ",:"
//field: 2
//returns pointer to "brown"
const char* const GSMbase::sendRecATCommandSplit(const char* theMessageMelleMel, 
const char* _delimiters, uint16_t _field){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return	(parseSplit(const_cast<char*>(catchTelitData()),_delimiters,_field));		//return data

}
//////////////////////////////////////////////////////////////////////SEND FUNCS*





//////////////////////////////////////////////////////////////////////PARSE FUNCS
//finds the objectOfDesire string in theString if it is ! a NULL pointer
bool GSMbase::parseFind(const char* const theString
,const char* objectOfDesire){
	if (!theString) return 0;                       // If we get a NULL pointer bail	
return strstr(theString,objectOfDesire);
}

//Main function which retrives data from serial buffer and puts it into 
//fullData, which has class scope.
const char* const GSMbase::catchTelitData(uint32_t _timeout, bool quickCheck){
	
	// block wait for reply
	uint64_t startTime = millis();			//9600/1000= 9.6bits per milli 6 milles ~6 bytes
	while (telitPort.available() < 1){ 		// smallest message "<CR><LF>OK<CR><LF>"    
		if((millis() - startTime) > 
		_timeout)return 0; 			// timed out bad message
	}

	//If serial data, get mem and fill 
	//with telitData.
	free(fullData);                  		//If it has been allocated Free the memory
	fullData = (char*) malloc(sizeof(char));  	//Get memory, call free or it blows real quick
	if (fullData == NULL)return 0;             	//If we couldn't get mem
	
	uint16_t dataPos=0;
	uint16_t baudDelay=60;				//if no data in 60 milli sendings done
	while (1){
		fullData[dataPos] = telitPort.read();	//Read out serial register
DebugPort->write(fullData[dataPos]);
		fullData = (char*)(realloc(fullData,
		(++dataPos + 1) * sizeof(char)));	//Re alloc fullData
		
		if (fullData == NULL)return 0; 		//If we had a bad realloc blow up
	
		startTime = millis();			
		while (telitPort.available() < 1){ 			
		if((millis() - startTime) > 
		baudDelay){goto doneReceive;} 		// if no data in x time goto doneReceive
		}

	}
doneReceive:
	fullData[dataPos]= '\0';		//NULL for a string
	
	if(quickCheck){
	if (parseFind(fullData, "\r\nOK\r\n")){ DebugPort->write("G\n"); return fullData;} 	//return fullData
	else if (parseFind(fullData,"ERROR")){ DebugPort->write("B\n"); return 0;}   	//return NULL
	else return 0;		
	}

	//else telitPort.flush();}			//bad flush all
//DebugPort->write("FULL DATA: \n");
//DebugPort->write(fullData);
//DebugPort->write(" END\n");

return fullData;
}

//Sends AT command parses reply, makes string from the passed in strings
//EG. 
//input string: "the quick brown blah blah"
//_start string: "the"
//_end string: "blah"
//returns a pointer to "quick brown"
const char* const GSMbase::parseData(const char* const theString,const char* start,
const char* end){
//DebugPort->write("parseData\n ");
	if (!theString) return 0;                       // If we get a NULL pointer bail	

	size_t startSize = strlen(start);		// get size of string 
	char* startP = strstr (theString,start);        // looks for string gives pointer including look
	startP+=startSize;                              // offset (gets rid of delim)
	char* endP = strstr ((startP),end);             // starts at startP looks for END string
	
	
	free(parsedData);          			// if it has been allocated, Free the memory
	parsedData = (char*) malloc(sizeof(char));      // get memory, call free or it blows real quick
	if (parsedData == NULL) return 0;            	// if we couldn't get mem
	
	uint16_t dataPos=0;
	while ( startP != endP ){			// grab between starP and endP
		parsedData[dataPos]= *startP++;
		parsedData = (char*)
		(realloc(parsedData,(++dataPos + 1)
		 * sizeof(char)));                	// re allocate mem
		if (parsedData == NULL) return 0;     	// if we had a bad realloc die quietly
	}
	parsedData[dataPos]= '\0';                      // NULL to make a proper string
return parsedData;					// gives back what it can. parsData has class scope.
}

//Sends AT command splits data according to delimeter
//EG
//input string: "the, quick: brown, blah: blah,"
//_delimiters string ",:"
//field: 2
//returns pointer to "brown"
const char* const GSMbase::parseSplit(const char* const theString,
const char* delimiters,uint16_t field){
	if (!theString) return 0;  			// if not a NULL pointer 

	char * temp;					// you have to use a local scope char array,	
	char deadStr[strlen(theString)+1];		// or you get a bad memory leak.
	strcpy(deadStr, theString);			// don't change this unless you are careful.
	temp = strtok (deadStr,delimiters);		// split here first
	for(uint16_t i=0; i<field;++i){
	temp = strtok (NULL,delimiters); 		// use NULL to keep spliting same string
     	}

	free(parsedData);          			// if it has been allocated Free the memory
	parsedData =  (char*) malloc(sizeof(char)* (strlen(temp)+1));   // get mem +'\0'
	if (parsedData == NULL) return 0;    				// If we get a NULL pointer 
	strcpy(parsedData,temp);                                    	// copy to parsedData, it has class scope


return parsedData;
}
//////////////////////////////////////////////////////////////////////PARSE FUNCS*
 

//////////////////////////////////////////////////////////////////////INIT FUNCS
//This is the only function to be re written for arduino
//you would need to include the wiring.h and binary.h 
//in header file and #define _cplusplus
#define OnOffPin PA0
bool GSMbase::turnOn(){
	if(sendRecQuickATCommand("AT")) return 1;		// the power is already on
	setOutput(DDRA,OnOffPin); 				// set direction register pin
	uint64_t startTime; 
	while(1){
		outputHigh(PORTA,OnOffPin); 			// bring pin high
	 	startTime = millis();		
		while ((millis() - startTime) < 3000); 		// block 3 seconds
		outputLow(PORTA,OnOffPin);			// bring pin low

		startTime = millis();		
		while ((millis() - startTime) < 10000);		// block 10 seconds
		if(sendRecQuickATCommand("AT")) return 1;	//set no echo if you get a OK we are ON!
DebugPort->write("stuck in ON");
		}
return 1;							//should never get here
}


//Turns off Telit
bool GSMbase::turnOff(){
	uint64_t startTime;
	while(1){
		
		if(sendRecQuickATCommand("AT#SHDN")){			//send off command
			startTime = millis();
			while (((millis() - startTime) < 10000)); 	//block 10 second
			if(!sendRecQuickATCommand("AT"))return 1;
DebugPort->write("stuck in OFF");
		}else if(!sendRecQuickATCommand("AT"))return 1;		//double check
	}		
DebugPort->write("stuck in OFF outside");


return 1;
}
//Used to init Telit to right settings, code doesn't work if not used.
bool GSMbase::init(uint16_t _band){
DebugPort->write("initalizing");

	if(!sendRecQuickATCommand("ATE0"))return 0;		//set no echo
	if(!sendRecQuickATCommand("ATV1"))return 0;		//set verbose mode
	if(!sendRecQuickATCommand("AT&K0"))return 0;		//set flow control off
	if(!sendRecQuickATCommand("AT+IPR=0"))return 0;		//set autoBaud (default not really needed)
	if(!sendRecQuickATCommand("AT+CMEE=2"))return 0;	//set exended error report
	switch(_band){
	case 0: if(!sendRecQuickATCommand("AT#BND=0"))return 0;	//0 - GSM 900MHz + DCS 1800MHz
	case 1: if(!sendRecQuickATCommand("AT#BND=1"))return 0;	//1 - GSM 900MHz + PCS 1900MHz
	case 2: if(!sendRecQuickATCommand("AT#BND=2"))return 0; //2 - GMS 850MHz + DCS 1800MHz 
	case 3: if(!sendRecQuickATCommand("AT#BND=3"))return 0; //3 - GMS 850MHz + PCS 1900MHz
	}

return 1;
}

//////////////////////////////////////////////////////////////////////INIT FUNCS*




