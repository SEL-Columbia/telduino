#include <string.h>
#include "arduino/WProgram.h"
#include "Select/select.h"
#include "circuit.h"
#include "calibration.h"
#include "ReturnCode/returncode.h"
#include "ADE7753/ADE7753.h"

#define dbg Serial1
/**
    Calibrate circuit interactively using serial port. 
	This function leaves the circuit off after completion.
	@warning need to finish VAslope/offset and WSlope/offset
	@warning this assumes that there is a gain of 1 on CH2OS
  */
int8_t calibrateCircuit(Circuit *c)
{
	//Clear values which need to be calibrated
	c->chIos = c->chVos = c->IRMSoffset = c->VRMSoffset = 0;
	c->VAoffset = c->Woffset = 0;
	c->IRMSslope = c->VRMSslope = c->VAslope = c->Wslope = 1;
	ifnsuccess(Cprogram(c)) dbg.println("Clearing failed in calibrateCircuit");

	int32_t regData;
	//The *Meas values are in mV or mA when read from the user
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

	//Calibrate low level channel offsets
	CsetOn(c,false);
	dbg.print("Ground both lines on circuit\'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and press ENTER (\'\\r\') when done.");
	while (dbg.read() != '\r');
	CsetOn(c,true);

	//Set waveform mode to read voltage
	dbg.println("Configuring to read raw voltage.");
	ifnsuccess(retCode = ADEsetModeBit(WAVESEL_0,true)) {
		CsetOn(c,false);
		return retCode;
	}
	ifnsuccess(retCode = ADEsetModeBit(WAVESEL1_,true)) {
		CsetOn(c,false);
		return retCode;
	}
	
	//Read waveform and set CH2OS (voltage) +500mV/10322/LSB in WAVEFORM
	dbg.println("Setting voltage offset.");
	ifnsuccess(retCode = ADEgetRegister(WAVEFORM,&regData)) return retCode;
	dbg.print("CHVwaveform:");bg.println(regData);
	//regData = regData*500*100/10322/161; //(1.61mV/LSB in CH2OS) and 500/10322 in WAVEFORM
	regData = (regData*31549)>>20; 
	//The CHXOS maxes out at 2^4 as it is a 5 bit signed magnitude number
	if (regData > 15){
		regData = 15;
	} else if (regData < -15){
		regData= -15;
	}
	int8_t offset = (int8_t)regData;
	ifnsuccess(retCode = ADEsetCHXOS(2,&c->chIint,&offset)) return retCode;
	dbg.print("CHVoffset:"); dbg.println(offset);
	CsetOn(c,false);

	//Query user to place load for low V,high I measurement
	dbg.println("Low-voltage (120VAC 50Hz), high-current (.8A):");
	dbg.print("Attach a low-voltage source and a 150 Ohm load to circuit \'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and press ENTER (\'\r\') when done.");
	while (dbg.read() != '\r');

	CsetOn(c,true);
	dbg.println("Enter measured mV. Press ENTER when done:");
	if (CLgetInt(&dbg,&VlowMeas) == CANCELED) {
		CsetOn(c,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(VlowMeas,DEC);
	dbg.print("Enter measured mA. Press ENTER when done:");
	if (CLgetInt(&dbg,&IhighMeas) == CANCELED) {
		CsetOn(c,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(IhighMeas,DEC);

	//get VRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,8000);
	ifnsuccess(retCode){
		CsetOn(c,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(VRMS,&VlowCkt)) return retCode;

	//get IRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		CsetOn(c,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(IRMS,&IhighCkt)) return retCode;
	CsetOn(c,false);

	//Query user to place load for high V,low I measurement
	dbg.print("Please attach a high-voltage source (240VAC 50Hz) "); 
	dbg.print("and a 2.4 KOhm load to circuit (.1A) \'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and press ENTER when done.");
	while (dbg.read() != '\r');

	CsetOn(c,true);
	dbg.println("Enter measured mV:");
	if (CLgetInt(&dbg,&VhighMeas) == CANCELED) {
		CsetOn(c,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(VhighMeas,DEC);
	dbg.println("Enter measured mA:");
	if (CLgetInt(&dbg,&IlowMeas) == CANCELED) {
		CsetOn(c,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(IlowMeas,DEC);

	//get VRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		CsetOn(c,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(VRMS,&VhighCkt)) return retCode;

	//get IRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		CsetOn(c,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(IRMS,&IlowCkt)) {
		CsetOn(c,false);
		return retCode;
	}
	CsetOn(c,false);
	
	dbg.println("Computing offsets and slopes for VRMS and IRMS.");
	//From page 46 in the ADE data sheet
	c->VRMSoffset = (VhighMeas*VlowCkt-VlowMeas*VhighCkt)/(VlowMeas-VhighMeas);
	if (c->VRMSoffset > 0x7FF) {
		c->VRMSoffset = 0x7FF;
	} else if (c->VRMSoffset < -2048) {
		c->VRMSoffset = -2048;
	}

	//Square everything and pray that IlowMeas is less than 2^15
	int64_t I1Msq = IhighMeas*IhighMeas;
	int64_t I2Msq = IlowMeas*IlowMeas;
	int64_t I1Csq = IlowCkt*IlowCkt;
	int64_t I2Csq = IhighCkt*IhighCkt;
	c->IRMSoffset = (int32_t)(I1Msq*I2Csq-I2Msq*I1Csq)/(I2Msq - I1Msq);
	if (c->IRMSoffset > 0x7FF) {	//12 bit twos complement limits
		c->IRMSoffset = 0x7FF;
	} else if (c->IRMSoffset < -0x800) {
		c->IRMSoffset = -0x800;
	}
	
	c->IRMSslope = ((float)(IlowMeas-IhighMeas))/(IlowCkt-IhighCkt);
	c->VRMSslope = ((float)(VlowMeas-VhighMeas))/(VlowCkt-VhighCkt);
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
	ifnsuccess(retCode = Cprogram(c)) return retCode;
	ifnsuccess(retCode = CSSelectDevice(DEVDISABLE)) return retCode;
	dbg.println("Calibration Complete.");
	return retCode;
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
	char buff[32] = {'\0'};
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
int8_t CLgetInt(HardwareSerial *ser,int32_t *d)
{
	char buff[32] = {'\0'};
	do {
		while(nsuccess(CLgetString(ser,buff,sizeof(buff)))) {
			ser->println("Buffer overflow: command too long.");
		}
		if (sscanf(buff,"%ld",d)) {
			return SUCCESS;
		}
		if (!strcmp(buff,"cancel")) {
			return CANCELED;
		}
	} while(true);
}
