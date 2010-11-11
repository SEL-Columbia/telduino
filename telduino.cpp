#include "arduino/WProgram.h"

//JR-Cal
#include "SDRaw/sd_raw.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"
#include "Select/select.h"
#include "prescaler.h"

const ADEReg *aardvark[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };

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
			delay(500);
			setDbgLeds(YPAT);
			delay(500);
			setDbgLeds(GPAT);
			delay(500);
		}
	}
} 

void setup()
{
	//Get rid of stinking prescaler.
	setClockPrescaler(CLOCK_PRESCALER_1);
	

	//Enable level shifters
	pinMode(37, OUTPUT);
	digitalWrite(37,HIGH);

	//Setup serial port
	Serial2.begin(9600);

	//Blink leds
	initDbgTel();

	//test Shift registers
	initShiftRegister();
	//test muxers
	initDemux();

	//test select
	initSelect();


	delay(100);
	uint8_t data = 0b0000100;
	/*Serial2.println(data,BYTE);
	Serial2.println(writeData(8,0x0F,&data));
	delay(100);*/
	//Serial2.println(readData(8,0x3F,&data));
	delay(100);
	//Serial2.println(data,BYTE);
	if (data){
		//Serial2.println("Uber Alles");
	}

	sd_raw_init();
	analogWrite(2,128);
	//Is it problematic for this to be called repeatedly.
	SPI.begin();
	//SPI.setClockDivider(0);
	SPI.setDataMode(SPI_MODE1);
}

void loop(){
	Serial2.print("CLKPR:");
	Serial2.println(CLKPR, BIN);
	Serial2.print("SPCR:");
	Serial2.println(SPCR,BIN);
	Serial2.println("Start");
	setDbgLeds(GYRPAT);
	delay(500);
	setDbgLeds(0);
	delay(500);
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
	selectSPIDevice(SDCARD);
	struct sd_raw_info info = {0}; 
	sd_raw_get_info(&info);
	if(info.manufacturer || info.revision) {
		Serial2.println(info.manufacturer,BIN);
		Serial2.println(info.revision,BIN);
	} else { 
	Serial2.println("Nothing From SD Card Received");
	}
	delay(1000);
	*/

	/* */
	/* ADE*/
	//INIT SPI
	//SPI
	selectSPIDevice(20);
	#define regist VRMS
	byte data[3] = {0};
	byte out = readData(regist,data);
	Serial2.println(out);
	Serial2.println(data[0],BIN);
	Serial2.println(data[1],BIN);
	Serial2.println(data[2],BIN);
	delay(1000);
	selectSPIDevice(DEVDISABLE);

	Serial2.print("data[0]==data[1]:");
	Serial2.println(data[0]==data[1]);

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
	
	Serial2.println("Restart");
	Serial2.flush();
}
