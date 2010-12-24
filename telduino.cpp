#include "arduino/WProgram.h"

//JR-Cal
#include "SDRaw/sd_raw.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"
#include "Select/select.h"
#include "prescaler.h"


const ADEReg *regList[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };

char ctrlz = 26;

#define testChannel 20
#define CYCEND 0x04 //bit 2 of the Interrupt Status register
#define CYCMODE 0x80 //bit 7 of the MODE register


void setup();
void loop();

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
	setClockPrescaler(CLOCK_PRESCALER_1); //Get rid of prescaler.
	
	//Level shifters
	pinMode(37, OUTPUT);
	digitalWrite(37,HIGH);

	Serial2.begin(9600); //Serial port
	initDbgTel(); //Blink leds
	initShiftRegister(); //Shift registers
	initDemux(); //Muxers
	initSelect(); //Select
	SPI.begin(); //SPI
	sd_raw_init(); //SDCard

	//SPI.setClockDivider(0);
	
	//Set the ch1 digital integrator on
	//#define regist CH1OS

	Serial2.print("/n/nStart Program/n/n");
	
	CSSelectDevice(testChannel); //start SPI comm with the test device channel
	uint32_t data = 0;

	uint32_t ch1osVal = 0x00000000;
	ch1osVal |= (1 << 7);
	ch1osVal = ch1osVal << 24;
	writeData(CH1OS,&ch1osVal);
	byte out = readData(CH1OS,&data);
	data = data >> 24;
	Serial2.print("BIN CH1OS:");
	Serial2.println(data,BIN);

	//set the gain to 2 for channel 1 since the sensitivity appears to be 0.02157 V/Amp
	uint32_t gainVal = 0x00000000;
	gainVal |= 1;
	gainVal = gainVal << 24;
	writeData(GAIN,&gainVal);
	out = readData(GAIN,&data);
	data = data >> 24;
	Serial2.print("BIN GAIN:");
	Serial2.println(data,BIN);
	
	//Set the IRMSOS to 0d99 or 0x63. This is the measured offset value.
	uint32_t iRmsOsVal = 0x00000000;
	iRmsOsVal |= 0x63;
	iRmsOsVal = iRmsOsVal << 20;
	writeData(IRMSOS,&iRmsOsVal);
	out = readData(IRMSOS,&data);
	data = data >> 20; // note that this is a signed number. right now we are expecting a positive one
	Serial2.print("int IRMSOS:");
	Serial2.println(data);
	
	//Set the IRMSOS to -0d522 or 0x63. This is the measured offset value.
	uint32_t vRmsOsVal = 0x00000000;
	vRmsOsVal |= 0xDF6;
	vRmsOsVal = vRmsOsVal << 20;
	writeData(VRMSOS,&vRmsOsVal);
	out = readData(VRMSOS,&data);
	data = data >> 20; // note that this is a signed number. right now we are expecting a negative one
	Serial2.print("int VRMSOS:");
	Serial2.println(data);
	
	//set the number of cycles to wait before taking a reading
	uint32_t linecycVal = 0xC8;
	linecycVal = linecycVal << 16;
	writeData(LINECYC,&linecycVal);
	out = readData(LINECYC,&data);
	data = data >> 16; // 16 bits
	Serial2.print("int linecycVal:");
	Serial2.println(data);
	
	//read and set the CYCMODE bit on the MODE register
	uint32_t modeReg = 0;
	out = readData(MODE,&data);
	modeReg = data >> 16; // 16 bits
	Serial2.print("bin MODE register before setting CYCMODE:");
	Serial2.println(modeReg, BIN);
	modeReg |= CYCMODE;	 //set the line cycle accumulation mode bit
	writeData(MODE,&modeReg);
	out = readData(MODE,&data);
	modeReg = data >> 16; // 16 bits
	Serial2.print("bin MODE register after setting CYCMODE:");
	Serial2.println(modeReg, BIN);
	
	//reset the Interrupt status register
	out = readData(RSTSTATUS, &data);
	data = data >> 16; //need only 16 bits for the status
	Serial2.print("bin Interrupt Status Register:");
	Serial2.println(data, BIN);
	modeReg |= CYCMODE;
	
	
	CSSelectDevice(DEVDISABLE); //end SPI comm with the selected device	
	
} //end of setup section

//Declare the variables used in the loop
uint32_t data = 0;
uint32_t iRMS = 0;
uint32_t vRMS = 0;
uint32_t lineAccAppEnergy = 0;
uint32_t interruptStatus = 0;
byte out = 0;
uint32_t loopCounter = 0;

void loop()
{	
	//Set variables to 0
	interruptStatus = 0;
	
	//Read the Interrupt Status Register
	CSSelectDevice(testChannel);
	out = readData(STATUS, &data);
	interruptStatus = data >> 16; //need only 16 bits for the status
	if(loopCounter%128){
		Serial2.print("bin Interrupt Status Register:");
		Serial2.println(interruptStatus, BIN);
	}	//endif
	
	//if the CYCEND bit of the Interrupt Status Registers is flagged
	if(interruptStatus & CYCEND){
		
		setDbgLeds(GYRPAT);
		Serial2.println("START");
		
		data = 0;
		iRMS = 0;
		out = 0;		
		out = readData(IRMS,&data);
		data = data >> 8;
		Serial2.print("int IRMS:");
		Serial2.println(data);
		iRMS = data/161;//data*1000/40172/4;
		Serial2.print("mAmps IRMS:");
		Serial2.println(iRMS);
		
		data = 0;
		vRMS = 0;
		out = 0;		
		out = readData(VRMS,&data);
		data = data >> 8;
		Serial2.print("int VRMS:");
		Serial2.println(data);
		vRMS = data/9142;
		Serial2.print("Volts VRMS:");
		Serial2.println(vRMS);
		
		data = 0;
		lineAccAppEnergy = 0;
		out = 0;		
		out = readData(LVAENERGY,&data);
		data = data >> 8;
		Serial2.print("int Line Apparent Energy:");
		Serial2.println(data);
/*		iRMS = data/161;//data*1000/40172/4;
		Serial2.print("mAmps IRMS:");
		Serial2.println(iRMS);
*/
		
		delay(500);
		out = readData(RSTSTATUS, &data);
		interruptStatus = data >> 16; //need only 16 bits for the status
		Serial2.print("bin Interrupt Status Register before Reset:");
		Serial2.println(interruptStatus, BIN);
		
	} //end of if statement

	Serial2.println();
	Serial2.flush();
	CSSelectDevice(DEVDISABLE);
	
	setDbgLeds(0);
	loopCounter++;

} //end of main loop


//*****Commented Section Below*****	
	
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

/*
	int32_t iData = 0;
	CSSelectDevice(testChannel);
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
	/*
	Serial2.print("CLKPR:");
	Serial2.println(CLKPR, BIN);
	Serial2.print("SPCR:");
	Serial2.println(SPCR,BIN);
	Serial2.print("SPDR:");
	Serial2.println(SPDR,BIN);
	
	1000100011010 
	*/
	

