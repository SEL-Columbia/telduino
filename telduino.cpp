/*************AVR SOURCE FILES FOR GSM,SERIAL FUNCTIONALITY*************\
* 1%  									*
*  			Copyright (C) 2010  Justin Downs of GRounND LAB	*
*       								*
*   			www.GroundLab.cc				*
*     									*
* This program is free software: you can redistribute it and/or modify	*
* it under the terms of the GNU General Public License as published by	*
* the Free Software Foundation, either version 3 of the License, or	*
* at your option) any later version.					*
*									*
* This program is distributed in the hope that it will be useful,	*
* but WITHOUT ANY WARRANTY; without even the implied warranty of	*
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*
* GNU General Public License for more details.				*
*									*
* You should have received a copy of the GNU General Public License	*
* with this program.  If not, see <http://www.gnu.org/licenses/>.	*
\***********************************************************************/



#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "ioHelper.h"
#include "timer.h"
#include "gsmbase.h"
#include "gsmSMS.h"
#include "gsmGPRS.h"
#include "gsmMaster.h"


//****************************************8
//Basic debug helpers
void talk();	//lets you type and get reply to Telit through Atmel
void blink();	//a hello world blink test for the pins

//****************************************8
//base class tester functions
void talkReply();   	// you type commands to ATMEL serial and telit responce is catched by GSMbase function.
void sendCommand(); 	// shows a typical send command using GSMbase class
void GSMbaseTester();	// tests GSMbase member functions
void turnOnTester();	// cycles a on off sequence
		
//*****************************************8
//SMS derived class tester
void gsmSMSTester();	// test gsmSMS member functions

//*****************************************8
uint32_t millisWrapper();
uint32_t (*milP)() = &millisWrapper;	//used, so with arduino you can pass &millis() straight in.
uint32_t millisWrapper(){return Timer0.millis();}

//*****************************************8

GSMbase GSMb(Serial3,milP,&Serial2);	//GSMbase TELIT BASE FUNCTIONALITY
gsmSMS  GsmSMS(Serial3,milP,&Serial2);	//gsmSMS TELIT SMS
gsmGPRS GsmGPRS(Serial3,milP,&Serial2); //gsmGPRS TELIT GPRS
gsmMASTER GsmMASTER(Serial3,milP,&Serial2);//combine base SMS and GPRS

int main(void){
	uint32_t delayStart;
	
	sei(); 				//Enable interupts
	//only these are declared, to turn on other serial ports do so in serial.h
	Serial3.USART_Init(9600); 	//Telit serial
	Serial2.USART_Init(9600); 	//debug serial
	Serial2.write("\r\n\r\ntelduino power up\r\n");
	
	//GSMb.init(3);	// init Telit ***ALWAYS INIT AFTER SERIAL SETUP***
	Serial2.write("telit power up\r\n");
	GSMb.turnOn();
	int init = GsmMASTER.init(3); // init Telit derived class calls base init()
    if (init==1) {
        Serial2.write("GsmMaster.init successful\r\n");
    } else {
        Serial2.write("GsmMaster.init failed\r\n");
    }
	/*
	//GSMb.sendRecQuickATCommand("AT+CFUN?\r\n");
	Serial3.write("AT+CFUN?\r\n");
	delayStart = millisWrapper();
	while((millisWrapper()-delayStart) < 1000);	
	GSMb.catchTelitData();
	//GSMbaseTester();
	//GsmSMS.sendNoSaveCMGS("8323776861","yo");
	Serial3.write("AT+CSQ?\r\n");
	delayStart = millisWrapper();
	while((millisWrapper()-delayStart) < 1000);	
	GSMb.catchTelitData();
	char ctrlz = 26;
	Serial2.write("message start\r\n");
	Serial3.write("AT+CMGS=8323776861\r\n");
	GSMb.catchTelitData();
	Serial3.write("message from columbia telduino ");
	Serial3.write(__TIME__);
	Serial3.write("\r\n");
	Serial3.write(ctrlz);
	GSMb.catchTelitData();
	*/
	
	while(1){
		while (Serial2.available()>0){
			Serial3.write(Serial2.read());
		}
		while (Serial3.available()>0){
			Serial2.write(Serial3.read());
			
		}
		
/*
		GSMb.turnOn();
		Serial2.write("top of while loop\r\n");

        Serial2.write("text mode?\r\n");
		Serial3.write("CMGF=?\r\n");

		delayStart = millisWrapper();
		while((millisWrapper() - delayStart) < 2000);

		while (Serial3.available() > 0) {
			Serial2.write(Serial3.read());
		}
		
		Serial2.write("antenna strength?\r\n");
		Serial3.write("AT+CSQ\r\n");
		
		delayStart = millisWrapper();
		while((millisWrapper() - delayStart) < 2000);

		while (Serial3.available() > 0) {
			Serial2.write(Serial3.read());
		}

		Serial3.write("AT+CFUN?\r\n");
		
		delayStart = millisWrapper();
		while((millisWrapper() - delayStart) < 2000);
		
		while (Serial3.available() > 0) {
			Serial2.write(Serial3.read());
		}

		char ctrlz = 26;
		Serial2.write("message start\r\n");
		Serial3.write("AT+CMGS=8323776861\r\n");

		delayStart = millisWrapper();
		while((millisWrapper() - delayStart) < 2000);
		
		Serial3.write("message from columbia telduino ");
		//Serial3.write(__TIME__);
		Serial3.write("\r\n");
		Serial3.write(ctrlz);
		
		delayStart = millisWrapper();
		while((millisWrapper() - delayStart) < 2000);

		while (Serial3.available() > 0) {
			Serial2.write(Serial3.read());
		}

		delayStart = millisWrapper();
		while((millisWrapper() - delayStart) < 2000);
 */
	}
}


//////////////////////////////////////////////////////////////////////////TALKING
void talk(){

	while (Serial3.available()>0){
		Serial2.write(Serial3.read());
	}
	while (Serial2.available()>0){
		Serial3.write(Serial2.read());
	}
}

///////////////////////////////////////////////////////////////////////////SMS CLASS TESTERS
void gsmSMSTester(){
//CHECK CMGD gives a numerical list of all messages which are stored seperated by ','
//	Serial2.write( GsmSMS.checkCMGDList() );
//*ERASE CMGD (with arg) erases message at id(EG 1,2,3,4) arg
//	GsmSMS.deletMessagesCMGD("9");
//*READ CMGR read message with specified ID	
//	GsmSMS.readMessageCMGR("4");
//SEND CMGS sends up a SMS with no save;
//	GsmSMS.sendNoSaveCMGS("3473017780","hello world!");
//	uint32_t startTime=millisWrapper();
//	while((millisWrapper()-startTime) < 20000);		// hang out 
//SAVE CMGW saves a message to memory and CMSS sends saved message.
//	GsmSMS.sendSavedMessageCMSS(GsmSMS.saveMessageCMGW("3473017780","I remember!"));	
//	uint32_t startTime=millisWrapper();
//	while((millisWrapper()-startTime) < 20000);		// hang out 
//CHECK CPMS gets number of messages or total space availible in sim memory selected by arg.
//	Serial2.write( GsmSMS.getNumMesInMemCPMS(1) );
//CHECK CMGL gets and displays full info and content of all of specified type of message.
	Serial2.write( GsmSMS.readAllCMGL("REC READ") );
//
}




////////////////////////////////////////////////////////////////////////////BASE CLASS TESTERS
void turnOnTester(){
//TURN ON
	Serial2.write("start");	
	GSMb.turnOn();
	GSMb.init(3);
	while(!GSMb.checkCREG());  		// hang out till it is registered
	
	uint32_t startTime=millisWrapper();
	while((millisWrapper()-startTime) < 20000);	// hang out for at  least 20 to let the cell tower
							//know you mean it.

	Serial2.write( GSMb.checkCOPS() );	// see who we are linked to
	GSMb.turnOff();				// once we are done turn off
	Serial2.write("out of off");
	startTime=millisWrapper();
	while((millisWrapper()-startTime) < 60000);	// hang out a minute
}


void GSMbaseTester(){
	Serial2.write("begin GSMbaseTester\r\n");
	//CHECK CREG (NETWORK REGISTRATION)
	//	if (GSMb.checkCREG()) Serial2.write("REGISTERED");
	//	else Serial2.write("NOTREGISTERED");

	//CHECK COPS (NETWORK INFO)
	Serial2.write( GSMb.checkCOPS() );

	//CHECK GSN (SERIAL NUMBER)
	Serial2.write( GSMb.checkGSN() );

	//CHECK CSQ (signal strength)
	char tempTest[10];
	Serial2.write(itoa( GSMb.checkCSQ(), tempTest, 10 ));

	//CHECK MONI (ALL SURRONDING TOWER INFORMATION)
	//	Serial2.write( GSMb.checkMONI() );
	Serial2.write("finished GSMbaseTester\r\n");
}



const char* result=NULL;
void sendCommand(){
	result = GSMb.sendRecATCommandSplit("AT+CSQ","\r",0); 	//send command, parse the reply 
	//result = GSMb.sendRecATCommand("AT+CSQ");
	if (!result) Serial2.write("returned bad pointer\n"); 	//check to see if it is good
	Serial2.write("Return results: ");
	Serial2.write(result);
	Serial2.write("\n");
	Serial2.write("FullData main loop: ");
	Serial2.write(GSMb.getFullData());			//Even though you parsed data you
								//can get the full reply by calling
								//getFullData before the next sendRec
								//call to the Telit
	Serial2.write("END\n");
	while(!Timer0.delay(4000));	//from my timer
	

	result = GSMb.sendRecATCommandSplit("AT+COPS=?","\r",0);
	//result = GSMb.sendRecATCommand("AT+COPS=?");
	if (!result) Serial2.write("returned bad pointer2\n");
	Serial2.write("Return2 results: ");
	Serial2.write(result);
	Serial2.write("\n");
	Serial2.write("FullData2 main loop: ");
	Serial2.write(GSMb.getFullData());
	Serial2.write("END2\n");

}


void talkReply(){
	while (Serial2.available()>0){
		Serial3.write(Serial2.read());
	}
	if (Serial3.available()>0){
		GSMb.catchTelitData(1000);
		Serial2.write(GSMb.getFullData());
		
	}
}

#define LED PB7
void blink(){
setOutput(DDRB,LED);

outputHigh(PORTB,LED);
	uint32_t startTime=millisWrapper();
	while((millisWrapper()-startTime) < 2000);
outputLow(PORTB,LED);
	startTime=millisWrapper();
	while((millisWrapper()-startTime) < 2000);
}
