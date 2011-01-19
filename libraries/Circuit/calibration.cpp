#include <string.h>
#include "arduino/WProgram.h"
#include "Select/select.h"
#include "circuit.h"
#include "calibration.h"
#include "ReturnCode/returncode.h"

#define dbg Serial1
/**
    Calibrate circuit interactively using serial port.
	@warning this assumes that there is a gain of 1 on CH2OS
  */
int8_t calibrateCircuit(Circuit *c)
{
	int32_t regData;
	//The *Meas values are in mV or mA when read form the user
	int32_t VlowCkt,VlowMeas;
	int32_t VhighCkt,VhighMeas;
	int32_t IlowCkt,IlowMeas;
	int32_t IhighCkt,IhighMeas;
	CSSelectDevice(c->circuitID);
	
	//Check to see if the circuit can be enabled.
	int8_t retCode; 
	ifnsuccess(retCode = Cenable(c,true)) {
		dbg.print("Circuit could not be enabled:");
		dbg.println(RCstr(retCode));
		return retCode;
	}

	//Calibrate offsets
	CsetOn(c,false);
	dbg.print("Ground both lines on circuit\'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and press ENTER (\'\r\') when done.");
	while (dbg.read() != '\r');
	CsetOn(c,true);

	//Set waveform mode to read voltage
	dbg.println("Configuring to read voltage.");
	ifnsuccess(retCode = ADEsetModeBit(WAVESEL_0,true)) return retCode;
	ifnsuccess(retCode = ADEsetModeBit(WAVESEL1_,true)) return retCode;
	
	//Read waveform and set CH2OS (voltage) +500mV/10322/LSB in WAVEFORM
	dbg.println("Setting voltage offset.");
	ifnsuccess(retCode = ADEgetRegister(WAVEFORM,&regData)) return retCode;
	regData = regData*500*100/10322/161; //(1.61mV/LSB in CH2OS)
	//The CHXOS maxes out at 2^4 as it is a 5 bit signed magnitude number
	if (regData > 16){
		regData = 16;
	} else if (regData < -16){
		regData= -16;
	}
	ifnsuccess(retCode = ADEsetCHXOS(2,&c->chIint,&regData)) return retCode;

	//Turn off circuit
	CsetOn(c,false);
	//Query user to place load for low V,high I measurement
	dbg.println("Low-voltage (120VAC 50Hz), high-current (.8A):");
	dbg.print("Attach a low-voltage source and a 150 Ohm load to circuit \'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and press ENTER (\'\r\') when done.");
	while (dbg.read() != '\r');
	CsetOn(c,true);

	//get VRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(VRMS,&VlowCkt)) return retCode;

	//get IRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(IRMS,&IhighCkt)) return retCode;

	dbg.println("Enter measured mV. Press ENTER when done:");
	if (CLgetInt(&dbg,&VlowMeas) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported by user:");
	dbg.println(VlowMeas,DEC);
	dbg.println("Enter measured mA. Press ENTER when done:");
	if (CLgetInt(&dbg,&IhighMeas) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported by user:");
	dbg.println(IhighMeas,DEC);
	CsetOn(c,false);

	//Query user to place load for high V,low I measurement
	dbg.println("High-voltage and low-current (.1A).");
	dbg.print("Please attach a low-voltage source (240VAC 50Hz) "); 
	dbg.print("and a 2.4 KOhm load to circuit \'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and press ENTER when done.");
	while (dbg.read() != '\r');
	CsetOn(c,true);

	dbg.println("Enter measured mV:");
	if (CLgetInt(&dbg,&VhighMeas) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported by user:");
	dbg.println(VhighMeas,DEC);
	dbg.println("Enter measured mA:");
	if (CLgetInt(&dbg,&IlowMeas) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported by user:");
	dbg.println(IlowMeas,DEC);

	
	/* pts are 
	Need to derive:
	IRMSoffset the IRMS offset register is 2^15 times one bit in IRMS so
		so the offset must be divided by 2^15
	IRMSSlope  this converts to engineering units
	Assuming no corrections are needed to chXos 
		we want to find the inverse of r(m) = (r1-r0)/(m0-m1)*m + 
			offsetIRMS*2^15, where m is the measured value and r 
			is the reported value
	VRMSoffset
	VRMSSlope
	chVos
	chIos
	VASlope
	*/
	CSSelectDevice(DEVDISABLE);
}

/**
    Calibrate circuit using another circuit as the reference.
	  */
void autoCalibrateCircuit(Circuit *c,const Circuit *reference)
{

}

int8_t CLgetString(HardwareSerial *ser,char *buff, size_t bSize)
{
	int i=0;
	do {
		while (ser->available() < 1);
		buff[i] = ser->read();
		//Handle backspace
		if (buff[i] == '\x7F' && i>=1) {
			buff[i] = '\0';
			ser->print('\b');
			ser->print(' ');
			ser->print('\b');
			i -= 1;
		} else if (buff[i]=='\x7F' && i<=0){
		} else {
			if (buff[i] == '\r') {
				break;
			}
			ser->print(buff[i]);
			i++;
		}
	} while (i < (bSize-2));
	if (i == bSize-2 && buff[i] != '\r') {
		return FAILURE;
	}
	buff[i] = '\0';
	return SUCCESS;
}
int8_t CLgetFloat(HardwareSerial *ser,float *f)
{
	char buff[64] = {'\0'};
	do {
		while(nsuccess(CLgetString(ser,buff,sizeof(buff)))) {
			ser->println("Buffer overflow: command too long.");
		}
		if (sscanf(buff,"%f",f)) {
			return SUCCESS;
		}
		if (!strcmp(buff,"cancel")) {
			return CANCELED;
		}
	} while(true);
}
int8_t CLgetInt(HardwareSerial *ser,int *d)
{
	char buff[64] = {'\0'};
	do {
		while(nsuccess(CLgetString(ser,buff,sizeof(buff)))) {
			ser->println("Buffer overflow: command too long.");
		}
		if (sscanf(buff,"%d",d)) {
			return SUCCESS;
		}
		if (!strcmp(buff,"cancel")) {
			return CANCELED;
		}
	} while(true);
}
