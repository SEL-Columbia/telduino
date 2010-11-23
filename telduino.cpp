#include "arduino/WProgram.h"

//JR-Cal
#include "SDRaw/sd_raw.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"
#include "Select/select.h"
#include "prescaler.h"


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "GSM/ioHelper.h"
#include "GSM/timer.h"
#include "GSM/gsmbase.h"
#include "GSM/gsmSMS.h"
#include "GSM/gsmGPRS.h"
#include "GSM/gsmMaster.h"

uint32_t millisWrapper();
uint32_t (*milP)() = &millisWrapper;	//used, so with arduino you can pass &millis() straight in.
uint32_t millisWrapper(){return Timer0.millis();}

GSMbase GSMb(Serial3,milP,&Serial2);	//GSMbase TELIT BASE FUNCTIONALITY
gsmSMS  GsmSMS(Serial3,milP,&Serial2);	//gsmSMS TELIT SMS
gsmGPRS GsmGPRS(Serial3,milP,&Serial2); //gsmGPRS TELIT GPRS
gsmMASTER GsmMASTER(Serial3,milP,&Serial2);//combine base SMS and GPRS

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

	delay(1000);

	setDbgLeds(0);delay(500);
}
