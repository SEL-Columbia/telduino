#include <string.h>
#include "arduino/WProgram.h"
#include "Select/select.h"
#include "circuit.h"
#include "calibration.h"
#include "ReturnCode/returncode.h"
#include "ADE7753/ADE7753.h"
#include "Strings/strings.h"

#define dbg Serial1
#define waitTime 8000
#define NAVG

#define EXITIFCANCELED(X)		\
	if ((X) == CANCELED) {		\
	    CsetOn(&cCal,false);	\
		dbg.println(CANCELEDSTR);\
		_retCode = CANCELED;	\
		return;	}				

#define EXITIFNOCYCLES()		\
	if (nsuccess(_retCode)) {	\
	    CsetOn(&cCal,false);	\
		dbg.println(NOACSTR);	\
		return;	 }				

/**
    Calibrate circuit interactively using serial port. 
	This function leaves the circuit off after completion.
	@warning need to finish VAslope/offset and WSlope/offset
	@warning this assumes that there is a gain of 1 on CH2OS
  */
void calibrateCircuit(Circuit *c)
{
	int32_t regData;
	//The *Meas values are in mV or mA when read from the user
	int32_t VlowCkt,VlowMeas;
	int32_t VhighCkt,VhighMeas;
	int32_t IlowCkt,IlowMeas;
	int32_t IhighCkt,IhighMeas;
	Circuit cCal = *c;				//In case of failure so settings are not lost.

	
	CsetOn(&cCal,true);
	//Clear values which need to be calibrated
	cCal.chIos = cCal.chVos = cCal.IRMSoffset = cCal.VRMSoffset = 0;
	cCal.VAoffset = cCal.Woffset = 0;
	cCal.IRMSslope = cCal.VRMSslope = cCal.VAslope = cCal.Wslope = 1;
	Cprogram(&cCal);
	ifnsuccess(_retCode) {
		dbg.println("Clearing failed in calibrateCircuit.");
		return;
	}

	//Calibrate low level channel offsets
	CSselectDevice(cCal.circuitID);
	/*
	//CsetOn(&cCal,false);
	dbg.print("Ground both lines on circuit \'");
	dbg.print(cCal.circuitID,DEC); 	dbg.println("\'."); dbg.print(PRESSENTERSTR);
	while (dbg.read() != '\r');

	//Set waveform mode to read voltage
	dbg.println("Configuring to read raw voltage.");
	ADEsetModeBit(WAVESEL_0,true);
	ifnsuccess(_retCode) return;
	ADEsetModeBit(WAVESEL1_,true);
	ifnsuccess(_retCode) return;
	ADEsetIrqEnBit(WSMP,true);	//The WAVEFORM register will not work without this.
	ADEsetIrqEnBit(CYCEND,true);//Just in case
	ifnsuccess(_retCode) return;
	dbg.println("WAVEFORM set to V, WSMP set to 1.");
	
	//Read waveform and set CH2OS (voltage) +500mV/10322/LSB in WAVEFORM
	dbg.println("Setting voltage offset.");
	//CsetOn(&cCal,true);
	CSstrobe();
	ADEgetRegister(RSTSTATUS,&regData); //reset interrupt
	ADEwaitForInterrupt(WSMP,waitTime);
	ifnsuccess(_retCode) {CsetOn(&cCal,false); dbg.println("Waiting for WSMP failed."); return;}
	ADEgetRegister(WAVEFORM,&regData);
	//CsetOn(&cCal,false); 
	ifnsuccess(_retCode) {dbg.println("get WAVEFORM failed"); return;}
	dbg.print("CHVwaveform:"); dbg.println(regData);
	//regData = regData*500*100/10322/161; //(1.61mV/LSB in CH2OS) and 500/10322 in WAVEFORM
	regData = (regData*31549)>>20; 
	//The CHXOS maxes out at 2^4 as it is a 5 bit signed magnitude number
	if (regData > 15){
		regData = 15;
	} else if (regData < -15){
		regData= -15;
	}
	int8_t offset = (int8_t)regData;
	cCal.chVos = offset;
	ADEsetCHXOS(2,&(cCal.chIint),&offset);
	ifnsuccess(_retCode) {dbg.println("set CHXOS 2 failed.");return;}
	dbg.print("CHVoffset:"); dbg.println(offset);
	*/

	//Query user to place load for low V,high I measurement
	dbg.print("Low-voltage (120VAC 50Hz), high-current (.72A) on ckt \'");
	dbg.print(cCal.circuitID,DEC); 	dbg.println("\'."); dbg.print(PRESSENTERSTR);
	while (dbg.read() != '\r');

	//GetmMV, mA from user for low voltage and low current
	//CsetOn(&cCal,true);
	dbg.println();
	dbg.println(MVQUERYSTR);
	EXITIFCANCELED(CLgetInt(&dbg,&VlowMeas));	
	dbg.println();
	dbg.print(REPORTEDSTR);
	dbg.println(VlowMeas,DEC);
	dbg.print(MAQUERYSTR);
	EXITIFCANCELED(CLgetInt(&dbg,&IhighMeas));	
	dbg.println();
	dbg.print(REPORTEDSTR);
	dbg.println(IhighMeas,DEC);

	//get VRMS from Ckt
	ADEgetRegister(RSTSTATUS,&regData); //reset interrupt
	ADEwaitForInterrupt(ZX,waitTime);
	EXITIFNOCYCLES();
	dbg.println("Saw ZX as 1");
	ADEwaitForInterrupt(ZX0,waitTime);
	//CsetOn(&cCal,false);
	EXITIFNOCYCLES();
	ADEgetRegister(VRMS,&VlowCkt);
	ifnsuccess(_retCode) { dbg.println("get VRMS Failed");return;}

	//get IRMS from Ckt
	ADEgetRegister(IRMS,&IhighCkt);
	ifnsuccess(_retCode) { dbg.println("get IRMS Failed");return;}
	dbg.print("ADELowVRMS: "); dbg.println(VlowCkt);
	dbg.print("ADEHighIRMS: "); dbg.println(IhighCkt);

	//Query user to place load for high V,low I measurement
	dbg.print("High-voltage (240VAC 50Hz), low-current (.025A) on ckt \'");
	dbg.print(cCal.circuitID,DEC);
	dbg.println("\'.");
	dbg.println(PRESSENTERSTR);
	while (dbg.read() != '\r');

	//CsetOn(&cCal,true);
	dbg.println(MVQUERYSTR);
	EXITIFCANCELED(CLgetInt(&dbg,&VhighMeas));
	dbg.println();
	dbg.print(REPORTEDSTR);
	dbg.println(VhighMeas,DEC);
	dbg.println(MAQUERYSTR);
	EXITIFCANCELED(CLgetInt(&dbg,&IlowMeas));	
	dbg.println();
	dbg.print(REPORTEDSTR);
	dbg.println(IlowMeas,DEC);

	//get VRMS from Ckt
	CSstrobe();
	ADEgetRegister(RSTSTATUS,&regData); //reset interrupt
	ADEwaitForInterrupt(ZX,waitTime);
	EXITIFNOCYCLES();
	dbg.println("Saw ZX as 1");
	ADEwaitForInterrupt(ZX0,waitTime);
	//CsetOn(&cCal,false);
	EXITIFNOCYCLES();
	ADEgetRegister(VRMS,&VhighCkt);
	ifnsuccess(_retCode) {dbg.println("get VRMS Failed");return;}

	//get IRMS from Ckt
	ADEgetRegister(IRMS,&IlowCkt);
	ifnsuccess(_retCode) {dbg.println("get IRMS Failed");return;}
	
	dbg.print("ADEHighVRMS: "); dbg.println(VhighCkt);
	dbg.print("ADELowIRMS: "); dbg.println(IlowCkt);
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
	cCal.IRMSoffset = (int32_t)(((I1Msq*I2Csq-I2Msq*I1Csq)/(I2Msq - I1Msq))>>15);
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
	Cprint(&dbg,&cCal);
	Cprogram(&cCal);
	ifnsuccess(_retCode) {
		dbg.println("Programming Failed.");
		return;
	} 
	CSselectDevice(DEVDISABLE);
	*c = cCal;						//Save new settings to *c
	dbg.println("Calibration Complete.");
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
/** 
	Gets a long int from the user.
	@returns retCode CANCELED if user cancels input.
  */
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

void CLwaitForZX10VIRMS() 
{
	int32_t regData,Vckt,Ickt;

	//get VRMS from Ckt
	CSstrobe();
	ADEgetRegister(RSTSTATUS,&regData); //reset interrupt
	_retCode = SUCCESS;
	ADEwaitForInterrupt(ZX,waitTime);
	dbg.println(RCstr(_retCode));
	dbg.print("Saw ZX as 1?:");
	dbg.println(RCstr(_retCode));
	_retCode = SUCCESS;
	ADEwaitForInterrupt(ZX0,waitTime);
	dbg.println(RCstr(_retCode));
	//CsetOn(&cCal,false);
	ADEgetRegister(VRMS,&Vckt);
	ifnsuccess(_retCode) {dbg.println("get VRMS Failed");return;}

	//get IRMS from Ckt
	ADEgetRegister(IRMS,&Ickt);
	ifnsuccess(_retCode) {dbg.println("get IRMS Failed");return;}
	
	dbg.print("ADEIRMS: "); dbg.println(Ickt);
	dbg.print("ADEVRMS: "); dbg.println(Vckt);

}
