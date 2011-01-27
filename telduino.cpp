/**
 *  \mainpage Telduino
 *
 *	\section Purpose
 *	This code runs on a custom avr board with hardware to control 20 relays and measurement circuits.
 *
 *	\section Implementation
 *  The code is a client to an embedded linux system that sends string commands over the serial port.
 *  These serial commands are executed by the telduino code and sent back to the linux box.
 */
 
#include <stdlib.h>
#include <errno.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <string.h>
#include <stdint.h>

//Metering Hardware
#include "arduino/WProgram.h"
#include "SPI/SPI.h"
#include "prescaler.h"
#include "ReturnCode/returncode.h"
#include "DbgTel/DbgTel.h"
#include "ADE7753/ADE7753.h"
#include "ShiftRegister/shiftregister.h"
#include "Demux/Demux.h"
#include "Select/select.h"
#include "sd-reader/sd_raw.h"
#include "Switches/switches.h"

//Metering logic
#include "Circuit/circuit.h"
#include "Circuit/calibration.h"

//definition of serial ports for debug, sheeva communication, and telit communication
#define debugPort Serial1
#define sheevaPort Serial2
#define telitPort Serial3

#define DEBUG_BAUD_RATE 9600
#define SHEEVA_BAUD_RATE 9600
#define TELIT_BAUD_RATE 115200
#define verbose 1


Circuit ckts[NCIRCUITS];

/*  Disables the watchdog timer the first chance the AtMega gets as recommended
	by Atmel.
 */
#include <avr/wdt.h>
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();
	return;     
}


int _testChannel = 1;

void setup();
void loop();
void softSetup();
void displayChannelInfo(); 
void displayEnabled(const int8_t enabledC[WIDTH]);
int8_t getChannelID();
void testHardware();
void parseBerkeley();
void parseColumbia();
String getValueForKey(String key, String commandString);
String getSMSText(String commandString);
void meter(String commandString);
void modem(String commandString);
String readSheevaPort();
String readTelitPort();
void chooseDestination(String destination, String commandString);
void turnOnTelit();

void setup()
{
	setClockPrescaler(CLOCK_PRESCALER_1);	//Disable prescaler.

	// start up serial ports
	debugPort.begin(9600);						//Debug serial
	telitPort.begin(TELIT_BAUD_RATE);		//Telit serial
	sheevaPort.begin(SHEEVA_BAUD_RATE);

	// write startup message to debug port
	debugPort.write("\r\n\r\ntelduino power up\r\n");
    debugPort.write("last compilation\r\n");
    debugPort.write(__DATE__);
    debugPort.write("\r\n");
    debugPort.write(__TIME__);
    debugPort.write("\r\n");

	turnOnTelit();				// set telit pin high

	pinMode(37, OUTPUT);		//Level shifters
	digitalWrite(37,HIGH);		//Level shifters
	initDbgTel();				//Blink leds
	SRinit();					//Shift registers
	initDemux();				//Muxers
	initSelect();				//Select Circuit
	sd_raw_init();				//SDCard
	SPI.begin();				//SPI

	SWallOff();
	_testChannel = 20;

	//Load circuit data from EEPROM
	uint8_t *addrEEPROM = 0;
	for (Circuit *c = ckts; c != &ckts[NCIRCUITS]+1; c++){
		//Cload(c,addrEEPROM);
		addrEEPROM += sizeof(Circuit);
	}
} //end of setup section


void loop()
{	
	parseBerkeley();
	//parseColumbia();
} //end of main loop

/**
 *	single character serial interface for interaction with telduino
 */
void parseBerkeley() 
{
	setDbgLeds(GYRPAT);
	debugPort.print(_testChannel,DEC);
	debugPort.print(" $");
	debugPort.println();
	while (debugPort.available() == 0);
	/**
	if (_testChannel > 10) {
		debugPort.print("\b");
	}
	debugPort.print("\b\b");
	*/

	// Look for incoming single character command on debugPort line
	// Capital letters denote write operations and lower case letters are reads
	if (debugPort.available() > 0) {
		char incoming = debugPort.read(); 
		if (incoming == 'A') {			//Write to ADE Register
			char buff[16] = {0};
			debugPort.print("Enter name of register to write:");
			CLgetString(&debugPort,buff,sizeof(buff));
			debugPort.println();
			
			int32_t regData = 0;
			for (int i=0; i < sizeof(regList)/sizeof(regList[0]); i++) {
				if (strcmp(regList[i]->name,buff) == 0){
					CSSelectDevice(_testChannel);
					debugPort.print("Current regData:");
					debugPort.print(RCstr(ADEgetRegister(*regList[i],&regData)));
					debugPort.print(":0x");
					debugPort.print(regData,HEX);
					debugPort.print(":");
					debugPort.println(regData,BIN);

					debugPort.print("Enter new regData:");
					if(CLgetInt(&debugPort,&regData) == CANCELED) break;	
					debugPort.println();
					debugPort.print(RCstr(ADEsetRegister(*regList[i],&regData)));
					debugPort.print(":0x");
					debugPort.print(regData,HEX);
					debugPort.print(":");
					debugPort.println(regData,DEC);
					CSSelectDevice(DEVDISABLE);
					break;
				} 
			}
		} else if (incoming == 'a') {		//Read ADE reg
			char buff[16] = {0};
			debugPort.print("Enter name of register to read:");
			CLgetString(&debugPort,buff,sizeof(buff));
			debugPort.println();

			int32_t regData = 0;
			for (int i=0; i < sizeof(regList)/sizeof(regList[0]); i++) {
				if (strcmp(regList[i]->name,buff) == 0){
					CSSelectDevice(_testChannel);
					debugPort.print("regData:");
					debugPort.print(RCstr(ADEgetRegister(*regList[i],&regData)));
					debugPort.print(":0x");
					debugPort.print(regData,HEX);
					debugPort.print(":");
					debugPort.println(regData,DEC);
					CSSelectDevice(DEVDISABLE);
					break;
				} 
			}
		} else if (incoming == 'C') {		//Change active channel
			_testChannel = getChannelID();	
		} else if (incoming == 'S') {		//Toggle channel circuit
			int8_t ID = getChannelID();		
			SWset(ID,!SWisOn(ID));
		} else if (incoming == 's') {		//Display switch state
			displayEnabled(SWgetSwitchState());	
		} else if (incoming == 'T') {		//Test basic functionality
			testHardware();
		} else if (incoming == 'R') {		//Hard Reset using watchdog timer
			wdt_enable((WDTO_4S));			
			Serial1.println("resetting in 4s.");
		} else if (incoming == 'O') {		//soft Reset using the Setup routine
			softSetup();					//Set calibration values for ADE
		} else if (incoming == 'o') {		//Read channel using Achintya's code
			displayChannelInfo();
		} else if (incoming == 'P') {		//Program values in ckts[] to ADE
			for (int i = 0; i < NCIRCUITS; i++) {
				Circuit *c = &(ckts[i]);
				int8_t retCode = Cprogram(c);
				debugPort.println(RCstr(retCode));
				debugPort.println("*****");
				ifnsuccess(retCode) {
					break;
				}
			}
		} else if(incoming == 'p') {		//Measure circuit values and print
			Circuit *c = &(ckts[_testChannel]);
			Cmeasure(c);
			CprintMeas(&debugPort,c);
			debugPort.println();
		} else if (incoming == 'L') {		//Run calibration routine on channel
			Circuit *c = &(ckts[_testChannel]);
			debugPort.println(RCstr(calibrateCircuit(c)));
		} else if (incoming == 'D') {		//Initialize ckts[] to safe defaults
			for (int i = 0; i < NCIRCUITS; i++) {
				Circuit *c = &(ckts[i]);
				CsetDefaults(c,i);
			}
			debugPort.println("Defaults set. Don't forget to program! ('P')");
		} else if (incoming == 'E') {		//Save data in ckts[] to EEPROM
			uint8_t *addrEEPROM = 0;
			for (Circuit *c = ckts; c != &ckts[NCIRCUITS]+1; c++){
				Csave(c,addrEEPROM);
				addrEEPROM += sizeof(Circuit);
			}
			debugPort.println("Save Complete");
		} else if (incoming=='e'){			//Load circuit data from EEPROM
			uint8_t *addrEEPROM = 0;
			for (Circuit *c = ckts; c != &ckts[NCIRCUITS]+1; c++){
				Cload(c,addrEEPROM);
				addrEEPROM += sizeof(Circuit);
			}
			debugPort.println("Load Complete");
		}
		else {								//Indicate received character
			debugPort.print("\n\rNot_Recognized:");
			debugPort.print(incoming,BIN);
			debugPort.print(":");
			debugPort.print("'");
			debugPort.print(incoming);
			debugPort.println("'");
		}
	}


	setDbgLeds(0);
}

void parseColumbia()
{
    if (verbose > 1) {
        debugPort.println("top of loop()");
        debugPort.println(millis());
    }
    
    String commandString;
    String destination;
	
    commandString = readSheevaPort();
    destination = getValueForKey("cmp", commandString);
    chooseDestination(destination, commandString);    
	
    String modemString = "";
    modemString = readTelitPort();
    modemString = modemString.trim();
    if (modemString.length() != 0) {
        String responseString = "";
        responseString += "cmp=mdm&text=";
        responseString += '"';
        responseString += modemString;
        responseString += '"';
        sheevaPort.println(responseString);
        
        debugPort.println("string received from telit");
        debugPort.println(modemString);
    }
}

void softSetup() 
{
	int32_t data = 0;

	debugPort.print("\n\n\rSetting Channel:");
	debugPort.println(_testChannel,DEC);
	
	CSSelectDevice(_testChannel); //start SPI comm with the test device channel
	//Enable Digital Integrator for _testChannel
	int8_t ch1os=0,enableBit=1;

	debugPort.print("set CH1OS:");
	debugPort.println(RCstr(ADEsetCHXOS(1,&enableBit,&ch1os)));
	debugPort.print("get CH1OS:");
	debugPort.println(RCstr(ADEgetCHXOS(1,&enableBit,&ch1os)));
	debugPort.print("enabled: ");
	debugPort.println(enableBit,BIN);
	debugPort.print("offset: ");
	debugPort.println(ch1os);

	//set the gain to 2 for channel _testChannel since the sensitivity appears to be 0.02157 V/Amp
	int32_t gainVal = 1;

	debugPort.print("BIN GAIN (set,get):");
	debugPort.print(RCstr(ADEsetRegister(GAIN,&gainVal)));
	debugPort.print(",");
	debugPort.print(RCstr(ADEgetRegister(GAIN,&gainVal)));
	debugPort.print(":");
	debugPort.println(gainVal,BIN);
	
	//Set the IRMSOS to 0d444 or 0x01BC. This is the measured offset value.
	int32_t iRmsOsVal = 0x01BC;
	ADEsetRegister(IRMSOS,&iRmsOsVal);
	ADEgetRegister(IRMSOS,&iRmsOsVal);
	debugPort.print("hex IRMSOS:");
	debugPort.println(iRmsOsVal, HEX);
	
	//WHAT'S GOING ON WITH THIS OFFSET? THE COMMENT DOESN'T MATCH THE VALUE
	//Set the VRMSOS to -0d549. This is the measured offset value.
	int32_t vRmsOsVal = 0x07FF;//F800
	ADEsetRegister(VRMSOS,&vRmsOsVal);
	ADEgetRegister(VRMSOS,&vRmsOsVal);
	debugPort.print("hex VRMSOS read from register:");
	debugPort.println(vRmsOsVal, HEX);
	
	//set the number of cycles to wait before taking a reading
	int32_t linecycVal = 200;
	ADEsetRegister(LINECYC,&linecycVal);
	ADEgetRegister(LINECYC,&linecycVal);
	debugPort.print("int linecycVal:");
	debugPort.println(linecycVal);
	
	//read and set the CYCMODE bit on the MODE register
	int32_t modeReg = 0;
	ADEgetRegister(MODE,&modeReg);
	debugPort.print("bin MODE register before setting CYCMODE:");
	debugPort.println(modeReg, BIN);
	modeReg |= CYCMODE;	 //set the line cycle accumulation mode bit
	ADEsetRegister(MODE,&modeReg);
	ADEgetRegister(MODE,&modeReg);
	debugPort.print("bin MODE register after setting CYCMODE:");
	debugPort.println(modeReg, BIN);
	
	//reset the Interrupt status register
	ADEgetRegister(RSTSTATUS, &data);
	debugPort.print("bin Interrupt Status Register:");
	debugPort.println(data, BIN);

	CSSelectDevice(DEVDISABLE); //end SPI comm with the selected device	
}

void displayChannelInfo() {
	int8_t retCode;
	int32_t val;
	uint32_t iRMS = 0;
	uint32_t vRMS = 0;
	uint32_t lineAccAppEnergy = 0;
	uint32_t lineAccActiveEnergy = 0;
	int32_t interruptStatus = 0;
	uint32_t iRMSSlope = 164;
	uint32_t vRMSSlope = 4700;
	uint32_t appEnergyDiv = 5;
	uint32_t energyJoules = 0;

	//Select the Device
	CSSelectDevice(_testChannel);
	
	//Read and clear the Interrupt Status Register
	ADEgetRegister(RSTSTATUS, &interruptStatus);
	
	if (0 /*loopCounter%4096*/ ){
		debugPort.print("bin Interrupt Status Register:");
		debugPort.println(interruptStatus, BIN);
		
	}	//endif
	
	//if the CYCEND bit of the Interrupt Status Registers is flagged
	debugPort.print("Waiting for next cycle: ");
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	debugPort.println(RCstr(retCode));

	ifsuccess(retCode) {
		setDbgLeds(GYRPAT);

		debugPort.print("_testChannel:");
		debugPort.println(_testChannel,DEC);

		debugPort.print("bin Interrupt Status Register:");
		debugPort.println(interruptStatus, BIN);
		
		//IRMS SECTION
		debugPort.print("mAmps IRMS:");
		debugPort.println( RCstr(ADEgetRegister(IRMS,&val)) );
		iRMS = val/iRMSSlope;//data*1000/40172/4;
		debugPort.println(iRMS);
		
		//VRMS SECTION
		debugPort.print("VRMS:");
		debugPort.println(RCstr(ADEgetRegister(VRMS,&val)));

		vRMS = val/vRMSSlope; //old value:9142
		debugPort.print("Volts VRMS:");
		debugPort.println(vRMS);

		
		//APPARENT ENERGY SECTION
		ADEgetRegister(LVAENERGY,&val);
		debugPort.print("int Line Cycle Apparent Energy after 200 half-cycles:");
		debugPort.println(val);
		energyJoules = val*2014/10000;
		debugPort.print("Apparent Energy in Joules over the past 2 seconds:");
		debugPort.println(energyJoules);
		debugPort.print("Calculated apparent power usage:");
		debugPort.println(energyJoules/2);
		
		//THIS IS NOT WORKING FOR SOME REASON
		//WE NEED TO FIX THE ACTIVE ENERGY REGISTER AT SOME POINT
		//ACTIVE ENERGY SECTION
		ifsuccess(ADEgetRegister(LAENERGY,&val)) {
			debugPort.print("int Line Cycle Active Energy after 200 half-cycles:");
			debugPort.println(val);
		} else {
			debugPort.println("Line Cycle Active Energy read failed.");
		}
		
/*		iRMS = data/161;//data*1000/40172/4;
		debugPort.print("mAmps IRMS:");
		debugPort.println(iRMS);
*/
		
		delay(500);
	} //end of if statement

	CSSelectDevice(DEVDISABLE);
}

int8_t getChannelID() 
{
	int32_t ID = -1;
	while (ID == -1) {
		debugPort.print("Waiting for ID (0-20):");		
		ifnsuccess(CLgetInt(&debugPort,&ID)) ID = -1;
		debugPort.println();
		if (ID < 0 || 20 < ID ) {
			debugPort.print("Incorrect ID:");
			debugPort.println(ID,DEC);
			ID = -1;
		} else {
			debugPort.println((int8_t)ID,DEC);
		}
	}
	return (int8_t)ID;
}

void testHardware() {
	int8_t enabledC[WIDTH] = {0};
	int32_t val;

	debugPort.print("\n\rTest switches\n\r");
	//Shut off/on all circuits
	for (int i =0; i < 1; i++){
		SWallOn();
		delay(50);
		SWallOff();
		delay(50);
	}
	//Start turning each switch on with 1 second in between
	for (int i = 0; i < WIDTH; i++) {
		enabledC[i] = 1;
		delay(1000);
		SWsetSwitches(enabledC);
	}
	delay(1000);
	SWallOff();

	//Test communications with each ADE
	for (int i = 0; i < 21; i++) {
		CSSelectDevice(i);
		
		debugPort.print("Can communicate with channel ");
		debugPort.print(i,DEC);
		debugPort.print(": ");

		int retCode = ADEgetRegister(DIEREV,&val);
		ifnsuccess(retCode) {
			debugPort.print("NO-");
			debugPort.println(RCstr(retCode));
		} else {
			debugPort.print("YES-DIEREV:");
			debugPort.println(val,DEC);
		}
		CSSelectDevice(DEVDISABLE);
	}
}
	
/** 
  Lists the state of the circuit switches.
  */
void displayEnabled(const int8_t enabledC[WIDTH])
{
	debugPort.println("Enabled Channels:");
	for (int i =0; i < WIDTH; i++) {
		debugPort.print(i);
		debugPort.print(":");
		debugPort.print(enabledC[i],DEC);
		if (i%4 == 3) {
			debugPort.println();
		} else {
			debugPort.print('\t');
		}
	}
	debugPort.println();
}

//JR needed to make compiler happy
extern "C" {
void __cxa_pure_virtual(void) 
{
	while(1) {
		setDbgLeds(RPAT);
		delay(332);
		setDbgLeds(YPAT);
		delay(332);
		setDbgLeds(GPAT);
		delay(332);
	}
}
}

/**
 *	given a String for the key value, this function returns the String corresponding
 *	to the value for the key by reading until the next '&' or the end of the string.
 */
String getValueForKey(String key, String commandString) {
    int keyIndex = commandString.indexOf(key);
    int valIndex = keyIndex + key.length() + 1;
    int ampersandIndex = commandString.indexOf("&",valIndex);
    // if ampersand not found, go until end of string
    if (ampersandIndex == -1) {
        ampersandIndex = commandString.length();
    }
    String val = commandString.substring(valIndex, ampersandIndex);
    return val;
}

/**
 *	this function is called when a cmp=mdm string is sent to the telduino.  the text 
 *	surrounded by parenthesis is returned.  this message will be sent to the modem as
 *	a raw string command.  
 */
String getSMSText(String commandString) {
    int firstDelimiterIndex = commandString.indexOf('(');
    int secondDelimiterIndex = commandString.indexOf(')', firstDelimiterIndex + 1);
    String smsText = commandString.substring(firstDelimiterIndex + 1, secondDelimiterIndex);
    return smsText;
}

/**
 *	this function takes care of parsing commands where cmp=mtr.
 */
void meter(String commandString) {
    String job = getValueForKey("job", commandString);
    String cid = getValueForKey("cid", commandString);
	
	int32_t val = 0;
	int32_t gainVal = 0x0F;
	
	// is there a better way to convert the cid string to int?
	char cidChar[3];
	cid.toCharArray(cidChar, 3);
	int icid = atoi(cidChar);
	
    if (verbose > 0) {
        debugPort.println();
        debugPort.println("entered void meter()");
        debugPort.print("executing job type - ");
        debugPort.println(job);
        debugPort.print("on circuit id - ");
        debugPort.println(cid);
        debugPort.println();
    }
    
	if (job == "con") {
		debugPort.println("execute con job");
		SWset(icid,1);
		debugPort.print("switch ");
		debugPort.print(icid, DEC);
		if (SWisOn(icid)) {
			debugPort.println(" is on");
		} else {
			debugPort.println(" is off");
		}
	}
	else if (job == "coff") {
		debugPort.println("execute coff job");
		SWset(icid,0);
		debugPort.print("switch ");
		debugPort.print(icid, DEC);
		if (SWisOn(icid)) {
			debugPort.println(" is on");
		} else {
			debugPort.println(" is off");
		}
	}
	else if (job == "read") {
		debugPort.println("reading circuit job");
		// actually do something here soon
		// read circuit energy or something using icid
	}
	else if (job == 'A') {
		_testChannel = icid;
		softSetup();
	}
	else if (job == 'c') {
		_testChannel = icid;
		displayChannelInfo();		
	}
	else if (job == 'T') {
		testHardware();
	}
	else if (job == 'R') {
		wdt_enable((WDTO_4S));
		debugPort.println("resetting in 4s.");
	}
}

/**
 *	this function takes care of parsing commands where cmp=mdm.
 */
void modem(String commandString) {
    String smsText = getSMSText(commandString);
    String job = getValueForKey("job", commandString);
	
	if (job == "ctrlz") {
		telitPort.print(26, BYTE);
		return;
	}

    if (verbose > 0) {
        debugPort.println();
        debugPort.println("entered void modem()");
        debugPort.print("sms text - ");
        debugPort.println(smsText);
        debugPort.println();
    }
	
	// send string to telit with a \r\n character appended to the string
	// todo - is it safer to send the char values for carriage return and linefeed?
    telitPort.print(smsText);
    telitPort.print("\r\n");
    
}

/**
 *	this function reads the sheevaPort (Serial2) for incoming commands
 *	and returns them as String objects.
 */
String readSheevaPort() {
    char incomingByte = ';';    
    String commandString = "";
    while ((sheevaPort.available() > 0) || ((incomingByte != ';') && (incomingByte != '\n'))) {
        incomingByte = sheevaPort.read();
        if (incomingByte != -1) {      
            if (verbose > 1) {
                debugPort.print(incomingByte);
            }
            commandString += incomingByte;
        }
        if (incomingByte == ';') {
            commandString = commandString.substring(0, commandString.length() - 1);
            break;
        }
    }   
    commandString = commandString.trim();
    return commandString;
}

/**
 *	this function reads the telitPort (Serial3) for incoming commands
 *	and returns them as String objects.
 */
String readTelitPort() {
	uint32_t startTime = millis();
    char incomingByte = '\n';
    String commandString = "";
    while ((telitPort.available() > 0) || (incomingByte != '\n')) {
        incomingByte = telitPort.read();
        if (incomingByte != -1) {      
            commandString += incomingByte;
        }
		if (commandString.indexOf(">") != -1) {
			return commandString;
		}
		if (millis() - startTime > 1000) {
			return commandString;
		}
    }
    return commandString;
}

/**
 *	based on the value for the cmp key, this calls the function
 *	meter if cmp=mtr
 *	and
 *  modem if cmp=mdm
 */
void chooseDestination(String destination, String commandString) {
    if (destination == "mtr") {
        meter(commandString);
    }
    else if (destination == "mdm") {
        modem(commandString);
    }
}

/**
 *	Pull telit on/off pin high for 3 seconds to start up telit modem
 */
void turnOnTelit() {
	pinMode(22, OUTPUT);
	digitalWrite(22, HIGH);
	delay(3000);
	digitalWrite(22, LOW);
}
