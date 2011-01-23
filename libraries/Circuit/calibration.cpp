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
	int8_t retCode; 
	int32_t regData;
	//The *Meas values are in mV or mA when read from the user
	int32_t VlowCkt,VlowMeas;
	int32_t VhighCkt,VhighMeas;
	int32_t IlowCkt,IlowMeas;
	int32_t IhighCkt,IhighMeas;
	Circuit cCal = *c;				//In case of failure so settings are not lost.

	CSSelectDevice(cCal.circuitID);
	
	//Clear values which need to be calibrated
	cCal.chIos = cCal.chVos = cCal.IRMSoffset = cCal.VRMSoffset = 0;
	cCal.VAoffset = cCal.Woffset = 0;
	cCal.IRMSslope = cCal.VRMSslope = cCal.VAslope = cCal.Wslope = 1;
	retCode = Cprogram(&cCal);
	ifnsuccess(retCode) {
		dbg.println("Clearing failed in calibrateCircuit");
		return retCode;
	}

	//Check to see if the circuit can be enabled.
	retCode = Cenable(&cCal,true);
	ifnsuccess(retCode) {
		dbg.print("Circuit could not be enabled:");
		dbg.println(RCstr(retCode));
		return retCode;
	}

	//Calibrate low level channel offsets
	CsetOn(&cCal,false);
	dbg.print("Ground both lines on circuit\'");
	dbg.print(cCal.circuitID,DEC);
	dbg.println("\' and press ENTER (\'\\r\') when done.");
	while (dbg.read() != '\r');
	CsetOn(&cCal,true);

	//Set waveform mode to read voltage
	dbg.println("Configuring to read raw voltage.");
	retCode = ADEsetModeBit(WAVESEL_0,true);
	ifnsuccess(retCode) {
		CsetOn(&cCal,false);
		return retCode;
	}
	retCode = ADEsetModeBit(WAVESEL1_,true);
	ifnsuccess(retCode) {
		CsetOn(&cCal,false);
		return retCode;
	}
	
	//Read waveform and set CH2OS (voltage) +500mV/10322/LSB in WAVEFORM
	dbg.println("Setting voltage offset.");
	retCode = ADEgetRegister(WAVEFORM,&regData);
	ifnsuccess(retCode) return retCode;
	dbg.print("CHVwaveform:");dbg.println(regData);
	//regData = regData*500*100/10322/161; //(1.61mV/LSB in CH2OS) and 500/10322 in WAVEFORM
	regData = (regData*31549)>>20; 
	//The CHXOS maxes out at 2^4 as it is a 5 bit signed magnitude number
	if (regData > 15){
		regData = 15;
	} else if (regData < -15){
		regData= -15;
	}
	int8_t offset = (int8_t)regData;
	retCode = ADEsetCHXOS(2,&(cCal.chIint),&offset);
	ifnsuccess(retCode) return retCode;
	dbg.print("CHVoffset:"); dbg.println(offset);
	CsetOn(&cCal,false);

	//Query user to place load for low V,high I measurement
	dbg.println("Low-voltage (120VAC 50Hz), high-current (.8A):");
	dbg.print("Attach a low-voltage source and a 150 Ohm load to circuit \'");
	dbg.print(cCal.circuitID,DEC);
	dbg.print("\' and press ENTER (\'\r\') when done.");
	while (dbg.read() != '\r');

	CsetOn(&cCal,true);
	dbg.println("Enter measured mV. Press ENTER when done:");
	if (CLgetInt(&dbg,&VlowMeas) == CANCELED) {
		CsetOn(&cCal,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(VlowMeas,DEC);
	dbg.print("Enter measured mA. Press ENTER when done:");
	if (CLgetInt(&dbg,&IhighMeas) == CANCELED) {
		CsetOn(&cCal,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(IhighMeas,DEC);

	//get VRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,8000);
	ifnsuccess(retCode){
		CsetOn(&cCal,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(VRMS,&VlowCkt)) return retCode;

	//get IRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		CsetOn(&cCal,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(IRMS,&IhighCkt)) return retCode;
	CsetOn(&cCal,false);

	//Query user to place load for high V,low I measurement
	dbg.print("Please attach a high-voltage source (240VAC 50Hz) "); 
	dbg.print("and a 2.4 KOhm load to circuit (.1A) \'");
	dbg.print(cCal.circuitID,DEC);
	dbg.print("\' and press ENTER when done.");
	while (dbg.read() != '\r');

	CsetOn(&cCal,true);
	dbg.println("Enter measured mV:");
	if (CLgetInt(&dbg,&VhighMeas) == CANCELED) {
		CsetOn(&cCal,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(VhighMeas,DEC);
	dbg.println("Enter measured mA:");
	if (CLgetInt(&dbg,&IlowMeas) == CANCELED) {
		CsetOn(&cCal,false);
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.println();
	dbg.print("Reported by user:");
	dbg.println(IlowMeas,DEC);

	//get VRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		CsetOn(&cCal,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(VRMS,&VhighCkt)) return retCode;

	//get IRMS from Ckt
	retCode = ADEwaitForInterrupt(CYCEND,4000);
	ifnsuccess(retCode){
		CsetOn(&cCal,false);
		dbg.println("Failed to sense cycles. Is a 120VAC 50Hz source connected?");
		return retCode;
	}
	ifnsuccess(retCode = ADEgetRegister(IRMS,&IlowCkt)) {
		CsetOn(&cCal,false);
		return retCode;
	}
	CsetOn(&cCal,false);
	
	dbg.println("Computing offsets and slopes for VRMS and IRMS.");
	//From page 46 in the ADE data sheet
	cCal.VRMSoffset = (VhighMeas*VlowCkt-VlowMeas*VhighCkt)/(VlowMeas-VhighMeas);
	if (cCal.VRMSoffset > 0x7FF) {
		cCal.VRMSoffset = 0x7FF;
	} else if (cCal.VRMSoffset < -2048) {
		cCal.VRMSoffset = -2048;
	}

	//Square everything and pray that IlowMeas is less than 2^15
	int64_t I1Msq = IhighMeas*IhighMeas;
	int64_t I2Msq = IlowMeas*IlowMeas;
	int64_t I1Csq = IlowCkt*IlowCkt;
	int64_t I2Csq = IhighCkt*IhighCkt;
	cCal.IRMSoffset = (int32_t)(I1Msq*I2Csq-I2Msq*I1Csq)/(I2Msq - I1Msq);
	if (cCal.IRMSoffset > 0x7FF) {	//12 bit twos complement limits
		cCal.IRMSoffset = 0x7FF;
	} else if (cCal.IRMSoffset < -0x800) {
		cCal.IRMSoffset = -0x800;
	}
	
	cCal.IRMSslope = ((float)(IlowMeas-IhighMeas))/(IlowCkt-IhighCkt);
	cCal.VRMSslope = ((float)(VlowMeas-VhighMeas))/(VlowCkt-VhighCkt);
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
	ifnsuccess(retCode = Cprogram(&cCal)) return retCode;
	ifnsuccess(retCode = CSSelectDevice(DEVDISABLE)) return retCode;
	*c = cCal;						//Save new settings to *c
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
		if (buff[0] == '0' && (buff[1] == 'x' || buff[1] == 'X')){
			if (sscanf(buff,"%lx",d)) return SUCCESS;
		} else { 
			if (sscanf(buff,"%ld",d)) return SUCCESS;
		}
		if (!strcmp(buff,"cancel")) {
			return CANCELED;
		}
	} while(true);
}
