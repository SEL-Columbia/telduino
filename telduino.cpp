#include "arduino/WProgram.h"

//JR-Cal
#include "SDRaw/sd_raw.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"
#include "Select/select.h"
#include "prescaler.h"


const ADEReg *regList[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };

char ctrlz = 26;

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
	//Get rid of prescaler.
	setClockPrescaler(CLOCK_PRESCALER_1);
	
	//Level shifters
	pinMode(37, OUTPUT);
	digitalWrite(37,HIGH);

	//Serial port
	Serial2.begin(9600);

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
}

void loop()
{
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
	selectSPIDevice(SDCARD);
	delay(1000);
	for(int i =0; i < 21; i++){
		selectSPIDevice(i);
		delay(500);
	}
	*/


	//SD Card
	/*
	SPI.setDataMode(SPI_MODE0);
	selectSPIDevice(SDCARD);
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
	#define regist VRMS
	uint32_t data = 0;
	selectSPIDevice(20);
	byte out = readData(regist,&data);
	selectSPIDevice(DEVDISABLE);

	Serial2.print("out:");
	Serial2.println(out,BIN);

	Serial2.print("int data:");
	Serial2.println(data);
	Serial2.print("BIN data:");
	Serial2.println(data,BIN);

	int32_t iData = 0;
	selectSPIDevice(20);
	ADEgetRegister(regist, &iData);
	selectSPIDevice(DEVDISABLE);

	Serial2.print("int iData:");
	Serial2.println(iData);
	Serial2.print("BIN iData:");
	Serial2.println(iData,BIN);
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

	delay(1000);

	setDbgLeds(0);delay(500);
}
