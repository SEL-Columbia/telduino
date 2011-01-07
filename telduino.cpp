#include <stdlib.h>
#include <errno.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <string.h>

//Metering Hardware
#include "arduino/WProgram.h"
#include "prescaler.h"
#include "ReturnCode/returncode.h"
#include "DbgTel/DbgTel.h"
#include "ADE7753/ADE7753.h"
#include "ShiftRegister/shiftregister.h"
#include "Demux/Demux.h"
#include "Select/select.h"
#include "sd-reader/sd_raw.h"
#include "Switches/switches.h"

//GSM Modem
#include "GSM/ioHelper.h"
#include "GSM/gsmbase.h"
#include "GSM/gsmSMS.h"
#include "GSM/gsmGPRS.h"
#include "GSM/gsmMaster.h"

#define dbg Serial1

//JR
#include <avr/wdt.h>
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();
	return;     
}


int _testChannel = 1;

//GSM functions 
uint32_t millisWrapper();
uint32_t (*milP)() = &millisWrapper;	//used, so with arduino you can pass &millis() straight in.
uint32_t millisWrapper(){return millis();}

GSMbase GSMb(Serial3,milP,&dbg);		//GSMbase TELIT BASE FUNCTIONALITY
gsmMASTER GsmMASTER(Serial3,milP,&dbg);	//combine base SMS and GPRS

gsmSMS  GsmSMS(Serial3,milP,&dbg);		//gsmSMS TELIT SMS
gsmGPRS GsmGPRS(Serial3,milP,&dbg);		//gsmGPRS TELIT GPRS


void setup();
void loop();
void softSetup();
void displayChannelInfo(); 
void displayEnabled(const int8_t enabledC[WIDTH]);
int8_t getChannelID();
void testHardware();


void setup()
{

	setClockPrescaler(CLOCK_PRESCALER_1);//Disable prescaler.


	dbg.begin(9600);		//Debug serial
	dbg.write("\r\n\r\ntelduino power up\r\n");
    dbg.write("last compilation\r\n");
    dbg.write(__DATE__);
    dbg.write("\r\n");
    dbg.write(__TIME__);
    dbg.write("\r\n");

	
	pinMode(37, OUTPUT);	//Level shifters
	digitalWrite(37,HIGH);	//Level shifters
	initDbgTel();			//Blink leds
	SRinit();				//Shift registers
	initDemux();			//Muxers
	initSelect();			//Select Circuit
	sd_raw_init();			//SDCard
	SPI.begin();			//SPI


	Serial3.begin(9600);	//Telit serial
	GSMb.turnOn();
	dbg.write("telit power up\r\n");
	int init = GsmMASTER.init(3); //Telit derived class calls base init()

	if (init==1) {
		dbg.write("GsmMaster.init successful\r\n");
	} else {
		dbg.write("GsmMaster.init failed\r\n");
	}


	SWallOff();
	_testChannel = 20;
	dbg.print("\r\nstart loop()\r\n");
} //end of setup section


void loop()

{	

	setDbgLeds(GYRPAT);
	// Look for incoming single character command on dbg line
	if (dbg.available() > 0) {
		char incoming = dbg.read(); 
		if (incoming == 'A') {
			softSetup();					//Set calibration values for ADE
		} else if (incoming == 'C') {
			_testChannel = getChannelID();	
		} else if (incoming == 'c') {
			displayChannelInfo();
		} else if (incoming == 'S') {
			int8_t ID = getChannelID();		//Toggle channel circuit
			//dbg.print("!SWisOn:");
			//dbg.print(!SWisOn(ID),DEC);
			//dbg.println(RCstr());
			SWset(ID,!SWisOn(ID));
		} else if (incoming == 's') {
			displayEnabled(SWgetSwitchState());	
		} else if (incoming == 'T'){
			testHardware();
		} else if (incoming == 'm') {
			GSMb.sendRecATCommand("AT+CCLK?");
			GSMb.sendRecATCommand("AT+CSQ");
		} else if (incoming == 'R') {
			wdt_enable((WDTO_4S));			//Do a soft reset
			Serial1.println("resetting in 4s.");
		} else if (incoming == 'F') {
			//Forward a command to the modem
			const int buffSize = 256;
			char command[buffSize] = {0};
			int8_t i = 0;
			dbg.println("Enter command:");
			do {
				while (dbg.available() < 1);
				command[i] = dbg.read();
				if (command[i] == '\x7F' && i>=1) {
					command[i] = '\0';
					dbg.print('\b');
					dbg.print(' ');
					dbg.print('\b');
					i -= 1;
				} else {
					dbg.print(command[i]);
					i++;
				}
			} while (command[i-1] != '\r' && \
					i < (buffSize-2));
			if (i == buffSize-2 && command[i] != '\r') {
				dbg.println("ERROR:INPUT_BUFFER_OVERFLOW");
			} else {
				command[i] = '\0'; //\r is added by sendRecATCommand
				wdt_enable(WDTO_8S);
				GSMb.sendRecQuickATCommand(command);
				wdt_disable();
			}
		}
		else {
			//Indicate received character
			dbg.print("\n\rNot_Recognized:");
			dbg.print(incoming,BIN);
			dbg.print(":");
			dbg.print("'");
			dbg.print(incoming);
			dbg.println("\'");
		}
	}
	setDbgLeds(0);
} //end of main loop


void softSetup() 
{
	int32_t data = 0;

	dbg.print("\n\n\rSetting Channel:");
	dbg.println(_testChannel,DEC);
	
	CSSelectDevice(_testChannel); //start SPI comm with the test device channel
	//Enable Digital Integrator for _testChannel
	int8_t ch1os=0,enableBit=1;

	dbg.print("set CH1OS:");
	dbg.println(RCstr(ADEsetCHXOS(1,&enableBit,&ch1os)));
	dbg.print("get CH1OS:");
	dbg.println(RCstr(ADEgetCHXOS(1,&enableBit,&ch1os)));
	dbg.print("enabled: ");
	dbg.println(enableBit,BIN);
	dbg.print("offset: ");
	dbg.println(ch1os);

	//set the gain to 2 for channel _testChannel since the sensitivity appears to be 0.02157 V/Amp
	int32_t gainVal = 1;

	dbg.print("BIN GAIN (set,get):");
	dbg.print(RCstr(ADEsetRegister(GAIN,&gainVal)));
	dbg.print(",");
	dbg.print(RCstr(ADEgetRegister(GAIN,&gainVal)));
	dbg.print(":");
	dbg.println(gainVal,BIN);
	
	//Set the IRMSOS to 0d444 or 0x01BC. This is the measured offset value.
	int32_t iRmsOsVal = 0x01BC;
	ADEsetRegister(IRMSOS,&iRmsOsVal);
	ADEgetRegister(IRMSOS,&iRmsOsVal);
	dbg.print("hex IRMSOS:");
	dbg.println(iRmsOsVal, HEX);
	
	//WHAT'S GOING ON WITH THIS OFFSET? THE COMMENT DOESN'T MATCH THE VALUE
	//Set the VRMSOS to -0d549. This is the measured offset value.
	int32_t vRmsOsVal = 0x07FF;//F800
	ADEsetRegister(VRMSOS,&vRmsOsVal);
	ADEgetRegister(VRMSOS,&vRmsOsVal);
	dbg.print("hex VRMSOS read from register:");
	dbg.println(vRmsOsVal, HEX);
	
	//set the number of cycles to wait before taking a reading
	int32_t linecycVal = 200;
	ADEsetRegister(LINECYC,&linecycVal);
	ADEgetRegister(LINECYC,&linecycVal);
	dbg.print("int linecycVal:");
	dbg.println(linecycVal);
	
	//read and set the CYCMODE bit on the MODE register
	int32_t modeReg = 0;
	ADEgetRegister(MODE,&modeReg);
	dbg.print("bin MODE register before setting CYCMODE:");
	dbg.println(modeReg, BIN);
	modeReg |= CYCMODE;	 //set the line cycle accumulation mode bit
	ADEsetRegister(MODE,&modeReg);
	ADEgetRegister(MODE,&modeReg);
	dbg.print("bin MODE register after setting CYCMODE:");
	dbg.println(modeReg, BIN);
	
	//reset the Interrupt status register
	ADEgetRegister(RSTSTATUS, &data);
	dbg.print("bin Interrupt Status Register:");
	dbg.println(data, BIN);

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
		dbg.print("bin Interrupt Status Register:");
		dbg.println(interruptStatus, BIN);
		
	}	//endif
	
	//if the CYCEND bit of the Interrupt Status Registers is flagged
	dbg.print("Waiting for next cycle: ");
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	dbg.println(RCstr(retCode));

	ifsuccess(retCode) {
		setDbgLeds(GYRPAT);

		dbg.print("_testChannel:");
		dbg.println(_testChannel,DEC);

		dbg.print("bin Interrupt Status Register:");
		dbg.println(interruptStatus, BIN);
		
		//IRMS SECTION
		dbg.print("mAmps IRMS:");
		dbg.println( RCstr(ADEgetRegister(IRMS,&val)) );
		iRMS = val/iRMSSlope;//data*1000/40172/4;
		dbg.println(iRMS);
		
		//VRMS SECTION
		dbg.print("VRMS:");
		dbg.println(RCstr(ADEgetRegister(VRMS,&val)));

		vRMS = val/vRMSSlope; //old value:9142
		dbg.print("Volts VRMS:");
		dbg.println(vRMS);

		
		//APPARENT ENERGY SECTION
		ADEgetRegister(LVAENERGY,&val);
		dbg.print("int Line Cycle Apparent Energy after 200 half-cycles:");
		dbg.println(val);
		energyJoules = val*2014/10000;
		dbg.print("Apparent Energy in Joules over the past 2 seconds:");
		dbg.println(energyJoules);
		dbg.print("Calculated apparent power usage:");
		dbg.println(energyJoules/2);
		
		//THIS IS NOT WORKING FOR SOME REASON
		//WE NEED TO FIX THE ACTIVE ENERGY REGISTER AT SOME POINT
		//ACTIVE ENERGY SECTION
		ifsuccess(ADEgetRegister(LAENERGY,&val)) {
			dbg.print("int Line Cycle Active Energy after 200 half-cycles:");
			dbg.println(val);
		} else {
			dbg.println("Line Cycle Active Energy read failed.");
		}
		
/*		iRMS = data/161;//data*1000/40172/4;
		dbg.print("mAmps IRMS:");
		dbg.println(iRMS);
*/
		
		delay(500);
	} //end of if statement

	CSSelectDevice(DEVDISABLE);
}

int8_t getChannelID() 
{
	int ID = -1;
	while (ID == -1) {
		dbg.print("Waiting for ID (0-20):");		
		char in[3] = {'\0'};
		while (dbg.available() == 0);
		in[0] = dbg.read();
		dbg.print(in[0]);
		while (dbg.available() == 0);
		in[1] = dbg.read();
		dbg.print(in[1]);
		ID = atoi(in);
		dbg.print(":");
		if (ID < 0 || 20 < ID || errno != 0) {
			dbg.print("Incorrect ID:");
			dbg.println(ID,DEC);
			ID = -1;
		} else {
			dbg.println((int8_t)ID,DEC);
		}
	}
	return (int8_t)ID;
}

void testHardware() {
	int8_t enabledC[WIDTH] = {0};
	int32_t val;

	dbg.print("\n\rTest switches\n\r");
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
		
		dbg.print("Can communicate with channel ");
		dbg.print(i,DEC);
		dbg.print(": ");

		int retCode = ADEgetRegister(DIEREV,&val);
		ifnsuccess(retCode) {
			dbg.print("NO-");
			dbg.println(RCstr(retCode));
		} else {
			dbg.print("YES-DIEREV:");
			dbg.println(val,DEC);
		}
		CSSelectDevice(DEVDISABLE);
	}
}
	
void displayEnabled(const int8_t enabledC[WIDTH])
{
	dbg.println("Enabled Channels:");
	for (int i =0; i < WIDTH; i++) {
		dbg.print(i);
		dbg.print(":");
		dbg.print(enabledC[i],DEC);
		if (i%4 == 3) {
			dbg.println();
		} else {
			dbg.print('\t');
		}
	}
	dbg.println();
}
	/* Select*/
	/*
	CSSelectDevice(SDCARD);
	delay(1000);
	for(int i =0; i < 21; i++){
		CSSelectDevice(i);
		delay(500);
	}
	*/
	
	//SD Card
	/*
	SPI.setDataMode(SPI_MODE0);
	CSSelectDevice(SDCARD);
	struct sd_raw_info info = {0}; 
	sd_raw_get_info(&info);
	if(info.manufacturer || info.revision) {
		setDbgLeds(GPAT);
		dbg.println("manufacturer");
		dbg.println(info.manufacturer,BIN);
		dbg.println("revision");
		dbg.println(info.revision,BIN);
	} else { 
		dbg.println("Nothing From SD Card Received");
	}
	delay(1000);
	*/


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

