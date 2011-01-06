<<<<<<< HEAD
#include "arduino/WProgram.h"

//JR-Cal
#include "sd-reader/sd_raw.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"
#include "Select/select.h"
#include "prescaler.h"


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "GSM/ioHelper.h"
#include "GSM/gsmbase.h"
#include "GSM/gsmSMS.h"
#include "GSM/gsmGPRS.h"
#include "GSM/gsmMaster.h"

uint32_t millisWrapper();
uint32_t (*milP)() = &millisWrapper;	//used, so with arduino you can pass &millis() straight in.
uint32_t millisWrapper(){return millis();}

GSMbase GSMb(Serial3,milP,&Serial2);	//GSMbase TELIT BASE FUNCTIONALITY
gsmMASTER GsmMASTER(Serial3,milP,&Serial2);//combine base SMS and GPRS

gsmSMS  GsmSMS(Serial3,milP,&Serial2);	//gsmSMS TELIT SMS
gsmGPRS GsmGPRS(Serial3,milP,&Serial2); //gsmGPRS TELIT GPRS


const ADEReg *regList[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };
=======
#include <stdlib.h>
#include <errno.h>
#include "arduino/WProgram.h"

//JR-Cal
#include "prescaler.h"
#include "ReturnCode/returncode.h"
#include "DbgTel/DbgTel.h"
#include "ADE7753/ADE7753.h"
#include "ShiftRegister/shiftregister.h"
#include "Demux/Demux.h"
#include "Select/select.h"
#include "SDRaw/sd_raw.h"
#include "Switches/switches.h"
>>>>>>> ArduinoHead


const ADEReg *regList[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };

int _testChannel = 1;
unsigned long long lastFire10 = 0;

void setup();
void loop();
<<<<<<< HEAD

//JR needed to make compiler happy
extern "C" {
#include "ShiftRegister/ShiftRegister.h"
#include "Demux/Demux.h"
	void __cxa_pure_virtual(void) {
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

void setup()
{
	
	Serial3.begin(9600); 	//Telit serial
	Serial2.begin(9600); 	//debug serial
	Serial2.write("\r\n\r\ntelduino power up\r\n");
	Serial2.write("last compilation\r\n");
	Serial2.write(__DATE__);
	Serial2.write("\r\n");
	Serial2.write(__TIME__);
	Serial2.write("\r\n");
	
	GSMb.turnOn();
	Serial2.write("telit power up\r\n");

	int init = GsmMASTER.init(3); // init Telit derived class calls base init()
	
    if (init==1) {
        Serial2.write("GsmMaster.init successful\r\n");
    } else {
        Serial2.write("GsmMaster.init failed\r\n");
    }
	
	//Get rid of prescaler.
	setClockPrescaler(CLOCK_PRESCALER_1);
	
	//Level shifters
	pinMode(37, OUTPUT);
	digitalWrite(37,HIGH);

	//Serial port
	//Serial2.begin(9600);

	//Blink leds
	initDbgTel();

	//Shift registers
	initShiftRegister();

	//Muxers
	initDemux();

	//Select
	initSelect();

	//SPI
	SPI.begin();

	//SDCard
	sd_raw_init();

	//SPI.setClockDivider(0);
	
	//Set the ch1 digital integrator on
	#define regist CH1OS
	
	CSSelectDevice(20);
	uint32_t ch1osVal = 0x00000000;
	ch1osVal |= (1 << 7);
	ch1osVal = ch1osVal << 24;
	writeData(regist,&ch1osVal);
	CSSelectDevice(DEVDISABLE);
	
	uint32_t data = 0;
	CSSelectDevice(20);
	byte out = readData(CH1OS,&data);
	CSSelectDevice(DEVDISABLE);

	Serial2.print("out:");
	Serial2.println(out,BIN);

	//Serial2.print("int data CH1OS:");
	//Serial2.println(data);
	Serial2.print("BIN data CH1OS:");
	Serial2.println(data,BIN);

	CSSelectDevice(20);
	out = readData(GAIN,&data);
	CSSelectDevice(DEVDISABLE);

	Serial2.print("out:");
	Serial2.println(out,BIN);

	//Serial2.print("int data:");
	//Serial2.println(data);
	Serial2.print("BIN data GAIN:");
	Serial2.println(data,BIN);
	
}

void loop()
{
	
	GSMb.sendRecATCommand("AT+CCLK?"); 
	GSMb.sendRecATCommand("AT+CSQ");
	
	setDbgLeds(GYRPAT);
	delay(500);
	Serial2.println("START");
	//setDbgLeds(GYPAT);

	
	/* Shift Reg.
	 * 
	int8_t first1 = 20;
	setEnabled(true);
	for (int8_t i = first1; i < first1 + 1; i++){
		shiftBit(true);
		//shiftBit(false);
		//latch();
	}
	for(int i = 0; i < first1; i++){
		shiftBit(false);
	}
	latch();
	delay(1000);
	clearShiftRegister();
	latch();
	*/

	/* Demux*/
	/*
	//muxSetEnabled(true);
	muxSelect(16);
	for (int i = 0; i < 21; i++){
		//muxSelect(i);
		//delay(1000);
	}*/

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
		Serial2.println("manufacturer");
		Serial2.println(info.manufacturer,BIN);
		Serial2.println("revision");
		Serial2.println(info.revision,BIN);
	} else { 
		Serial2.println("Nothing From SD Card Received");
	}
	delay(1000);
	*/
	

	/* ADE*/
	//INIT SPI
	//SPI
	
	
	uint32_t data = 0;
	CSSelectDevice(20);
	byte out = readData(IRMS,&data);
	CSSelectDevice(DEVDISABLE);

	Serial2.print("out:");
	Serial2.println(out,BIN);
	data = data >> 8;

	Serial2.print("int data IRMS:");
	Serial2.println(data);
	Serial2.print("BIN data IRMS:");
	Serial2.println(data,BIN);
	
	data = 0;
	CSSelectDevice(20);
	out = readData(VRMS,&data);
	CSSelectDevice(DEVDISABLE);

	Serial2.print("out:");
	Serial2.println(out,BIN);
	data = data >> 8;

	Serial2.print("int data VRMS:");
	Serial2.println(data);
	Serial2.print("BIN data VRMS:");
	Serial2.println(data,BIN);
	

	
/*	
	int32_t iData = 0;
	CSSelectDevice(20);
	ADEgetRegister(regist, &iData);
	CSSelectDevice(DEVDISABLE);

	Serial2.print("int iData:");
	Serial2.println(iData);
	Serial2.print("BIN iData:");
	Serial2.println(iData,BIN);
*/
	
	
	/*
	int8_t PAT = 0;
	if(data[0]){
		PAT |= GRNPAT;
	}
	if(data[1]){
		PAT |= YELPAT;
	}
	if(data[2]){
		PAT |=REDPAT;
	}
	setDbgLeds(PAT);
	delay(10000);
	*/
	
	Serial2.print("CLKPR:");
	Serial2.println(CLKPR, BIN);
	Serial2.print("SPCR:");
	Serial2.println(SPCR,BIN);
	Serial2.print("SPDR:");
	Serial2.println(SPDR,BIN);
	Serial2.println();
	Serial2.flush();

	delay(10000);

	setDbgLeds(0);delay(500);
=======
void softSetup();
void displayChannelInfo(); 
void displayEnabled(const int8_t enabledC[WIDTH]);
int8_t getChannelID();
void testHardware();

//JR needed to make compiler happy
void __cxa_pure_virtual(void) {
	while(1) {
		setDbgLeds(RPAT);
		delay(332);
		setDbgLeds(YPAT);
		delay(332);
		setDbgLeds(GPAT);
		delay(332);
	}
}

void setup()
{
	setClockPrescaler(CLOCK_PRESCALER_1); //Get rid of prescaler.

	Serial1.begin(9600); //Serial port
	Serial1.print("\n\rEXECUTING SETUP\n\r");
	
	//Level shifters
	pinMode(37, OUTPUT);
	digitalWrite(37,HIGH);

	initDbgTel(); //Blink leds
	SRinit(); //Shift registers
	initDemux(); //Muxers
	initSelect(); //Select Circuit
	sd_raw_init(); //SDCard
	SPI.begin(); //SPI

	SWallOff();
	_testChannel = 20;

	Serial1.print("\n\rStart loop()\n\r");
} //end of setup section


void loop()

{	
	// Look for incoming data on Serial1 line
	if (Serial1.available() > 0) {
		char incoming = Serial1.read(); 
		if (incoming == 'R') {
			softSetup(); //do a soft reset.
		} else if (incoming == 'C') {
			//set display channel
			_testChannel = getChannelID();
		} else if (incoming == 'c') {
			displayChannelInfo();
		} else if (incoming == 'S') {
			//Toggle channel circuit
			int8_t ID = getChannelID();
			//Serial1.print("!SWisOn:");
			//Serial1.print(!SWisOn(ID),DEC);
			//Serial1.println(RCstr());
			SWset(ID,!SWisOn(ID));
		} else if (incoming == 's') {
			displayEnabled(SWgetSwitchState());	
		} else if (incoming == 'T'){
			testHardware();
		} else {
			//Indicate received character
			Serial1.print("\n\rNot_Recognized: \'");
			Serial1.print(incoming);
			Serial1.println("\'");
		}
	}
	setDbgLeds(0);
} //end of main loop


void softSetup() 
{
	int32_t data = 0;

	Serial1.print("\n\n\rSetting Channel:");
	Serial1.println(_testChannel,DEC);
	
	CSSelectDevice(_testChannel); //start SPI comm with the test device channel
	//Turn on the Digital Integrator for _testChannel
	int8_t ch1os=0,enableBit=1;

	Serial1.print("set CH1OS:");
	Serial1.println(RCstr(ADEsetCHXOS(1,&enableBit,&ch1os)));
	Serial1.print("get CH1OS:");
	Serial1.println(RCstr(ADEgetCHXOS(1,&enableBit,&ch1os)));
	Serial1.print("enabled: ");
	Serial1.println(enableBit,BIN);
	Serial1.print("offset: ");
	Serial1.println(ch1os);

	//set the gain to 2 for channel _testChannel since the sensitivity appears to be 0.02157 V/Amp
	int32_t gainVal = 1;

	Serial1.print("BIN GAIN (set,get):");
	Serial1.print(RCstr(ADEsetRegister(GAIN,&gainVal)));
	Serial1.print(",");
	Serial1.print(RCstr(ADEgetRegister(GAIN,&gainVal)));
	Serial1.print(":");
	Serial1.println(gainVal,BIN);
	
	//Set the IRMSOS to 0d444 or 0x01BC. This is the measured offset value.
	int32_t iRmsOsVal = 0x01BC;
	ADEsetRegister(IRMSOS,&iRmsOsVal);
	ADEgetRegister(IRMSOS,&iRmsOsVal);
	Serial1.print("hex IRMSOS:");
	Serial1.println(iRmsOsVal, HEX);
	
	//WHAT'S GOING ON WITH THIS OFFSET? THE COMMENT DOESN'T MATCH THE VALUE
	//Set the VRMSOS to -0d549. This is the measured offset value.
	int32_t vRmsOsVal = 0x07FF;//F800
	ADEsetRegister(VRMSOS,&vRmsOsVal);
	ADEgetRegister(VRMSOS,&vRmsOsVal);
	Serial1.print("hex VRMSOS read from register:");
	Serial1.println(vRmsOsVal, HEX);
	
	//set the number of cycles to wait before taking a reading
	int32_t linecycVal = 200;
	ADEsetRegister(LINECYC,&linecycVal);
	ADEgetRegister(LINECYC,&linecycVal);
	Serial1.print("int linecycVal:");
	Serial1.println(linecycVal);
	
	//read and set the CYCMODE bit on the MODE register
	int32_t modeReg = 0;
	ADEgetRegister(MODE,&modeReg);
	Serial1.print("bin MODE register before setting CYCMODE:");
	Serial1.println(modeReg, BIN);
	modeReg |= CYCMODE;	 //set the line cycle accumulation mode bit
	ADEsetRegister(MODE,&modeReg);
	ADEgetRegister(MODE,&modeReg);
	Serial1.print("bin MODE register after setting CYCMODE:");
	Serial1.println(modeReg, BIN);
	
	//reset the Interrupt status register
	ADEgetRegister(RSTSTATUS, &data);
	Serial1.print("bin Interrupt Status Register:");
	Serial1.println(data, BIN);

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
		Serial1.print("bin Interrupt Status Register:");
		Serial1.println(interruptStatus, BIN);
		
	}	//endif
	
	//if the CYCEND bit of the Interrupt Status Registers is flagged
	Serial1.print("Waiting for next cycle: ");
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	Serial1.println(RCstr(retCode));

	ifsuccess(retCode) {
		setDbgLeds(GYRPAT);

		Serial1.print("_testChannel:");
		Serial1.println(_testChannel,DEC);

		Serial1.print("bin Interrupt Status Register:");
		Serial1.println(interruptStatus, BIN);
		
		//IRMS SECTION
		Serial1.print("mAmps IRMS:");
		Serial1.println( RCstr(ADEgetRegister(IRMS,&val)) );
		iRMS = val/iRMSSlope;//data*1000/40172/4;
		Serial1.println(iRMS);
		
		//VRMS SECTION
		Serial1.print("VRMS:");
		Serial1.println(RCstr(ADEgetRegister(VRMS,&val)));

		vRMS = val/vRMSSlope; //old value:9142
		Serial1.print("Volts VRMS:");
		Serial1.println(vRMS);

		
		//APPARENT ENERGY SECTION
		ADEgetRegister(LVAENERGY,&val);
		Serial1.print("int Line Cycle Apparent Energy after 200 half-cycles:");
		Serial1.println(val);
		energyJoules = val*2014/10000;
		Serial1.print("Apparent Energy in Joules over the past 2 seconds:");
		Serial1.println(energyJoules);
		Serial1.print("Calculated apparent power usage:");
		Serial1.println(energyJoules/2);
		
		//THIS IS NOT WORKING FOR SOME REASON
		//WE NEED TO FIX THE ACTIVE ENERGY REGISTER AT SOME POINT
		//ACTIVE ENERGY SECTION
		ifsuccess(ADEgetRegister(LAENERGY,&val)) {
			Serial1.print("int Line Cycle Active Energy after 200 half-cycles:");
			Serial1.println(val);
		} else {
			Serial1.println("Line Cycle Active Energy read failed.");
		}
		
/*		iRMS = data/161;//data*1000/40172/4;
		Serial1.print("mAmps IRMS:");
		Serial1.println(iRMS);
*/
		
		delay(500);
	} //end of if statement

	CSSelectDevice(DEVDISABLE);
}

int8_t getChannelID() 
{
	int ID = -1;
	while (ID == -1) {
		Serial1.print("Waiting for ID (0-20):");		
		char in[3] = {'\0'};
		while (Serial1.available() == 0);
		in[0] = Serial1.read();
		Serial1.print(in[0]);
		while (Serial1.available() == 0);
		in[1] = Serial1.read();
		Serial1.print(in[1]);
		ID = atoi(in);
		Serial1.print(":");
		if (ID < 0 || 20 < ID || errno != 0) {
			Serial1.print("Incorrect ID:");
			Serial1.println(ID,DEC);
			ID = -1;
		} else {
			Serial1.println((int8_t)ID,DEC);
		}
	}
	return (int8_t)ID;
}

void testHardware() {
	int8_t enabledC[WIDTH] = {0};
	int32_t val;

	Serial1.print("\n\rTest switches\n\r");
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
		
		Serial1.print("Can communicate with channel ");
		Serial1.print(i,DEC);
		Serial1.print(": ");

		int retCode = ADEgetRegister(DIEREV,&val);
		ifnsuccess(retCode) {
			Serial1.print("NO-");
			Serial1.println(RCstr(retCode));
		} else {
			Serial1.print("YES-DIEREV:");
			Serial1.println(val,DEC);
		}
		CSSelectDevice(DEVDISABLE);
	}
}
	
void displayEnabled(const int8_t enabledC[WIDTH])
{
	Serial1.println("Enabled Channels:");
	for (int i =0; i < WIDTH; i++) {
		Serial1.print(i);
		Serial1.print(":");
		Serial1.print(enabledC[i],DEC);
		if (i%4 == 3) {
			Serial1.println();
		} else {
			Serial1.print('\t');
		}
	}
	Serial1.println();
>>>>>>> ArduinoHead
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
		Serial1.println("manufacturer");
		Serial1.println(info.manufacturer,BIN);
		Serial1.println("revision");
		Serial1.println(info.revision,BIN);
	} else { 
		Serial1.println("Nothing From SD Card Received");
	}
	delay(1000);
	*/
