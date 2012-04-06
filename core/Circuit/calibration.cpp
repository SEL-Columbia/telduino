#include <string.h>
#include "arduino/WProgram.h"
#include "Select/select.h"
#include "circuit.h"
#include "calibration.h"
#include "ReturnCode/returncode.h"
#include "ADE7753/ADE7753.h"
#include "Strings/strings.h"
#include "Statistics/statistics.h"
#include "cfg.h"

#define waitTime 8000
#define NAVG 10

#define EXITIFCANCELED(X)		\
	if ((X) == CANCELED) {		\
	    CsetOn(&cCal,false);	\
		dbg.println();			\
		dbg.println(CANCELEDSTR);\
		_retCode = CANCELED;	\
		return false;	}				

#define EXITIFNOCYCLES()		\
	if (nsuccess(_retCode)) {	\
	    CsetOn(&cCal,false);	\
		dbg.println(NOACSTR);	\
		return false;	 }				

/**
	Used to gather measurement data interactively over the serial port (dbg). 
    Assumes a resistive load.
    VRMS and IRMS are in milliVolts/Amps
    VA is in Watts
	@warning Assumes 200 line cycles and 50hz
  */
int8_t getPoint(Circuit cCal, int32_t *VRMSMeas,int32_t *IRMSMeas, int32_t *VAMeas,
	 		               int32_t *VRMSCkt, int32_t *IRMSCkt,  int32_t *VACkt ) 
{
	//get VRMS from user
	dbg.println(MVQUERYSTR);
	EXITIFCANCELED(CLgetInt(&dbg,VRMSMeas));	
	dbg.println();
	dbg.print(REPORTEDSTR);
	dbg.println(*VRMSMeas,DEC);

	//get VRMS from Ckt
    // TODO average multiple measurements
    delay(2000); // Settling time according to ADE
    CwaitForZX10(&cCal);
	EXITIFNOCYCLES();
	ADEgetRegister(VRMS,VRMSCkt);
	ifnsuccess(_retCode) {dbg.println("get VRMS Failed");return false;}

	//get IRMS from user
	dbg.print(MAQUERYSTR);
	EXITIFCANCELED(CLgetInt(&dbg,IRMSMeas));	
	dbg.println();
	dbg.print(REPORTEDSTR);
	dbg.println(*IRMSMeas,DEC);
	*VAMeas = (*IRMSMeas)*(*VRMSMeas)/1000; //Was *2/1000

	//get IRMS from Ckt
    // TODO average multiple measurements
    delay(2000); // Settling time according to ADE
    CwaitForZX10(&cCal);
	EXITIFNOCYCLES();
	ADEgetRegister(IRMS,IRMSCkt);
	ifnsuccess(_retCode) {dbg.println("get IRMS Failed");return false;}
	dbg.print("ADEVRMS: "); dbg.println(*VRMSCkt);
	dbg.print("ADEIRMS: "); dbg.println(*IRMSCkt);

	//TODO For active power PHCAL attach reactive load
    
	//getVA from Ckt
	ADEgetRegister(RSTSTATUS,VACkt);
	ADEwaitForInterrupt(CYCEND,waitTime);
	EXITIFNOCYCLES();
	ADEgetRegister(LVAENERGY,VACkt);
	return true;
}

/**
    Calibrate circuit interactively using serial port. 
	First the output lines are grounded to digital ground. 
	Second a low voltage and high current source/load are attached and measured. 
	Third a high-voltage and low-current source/load are attached an measured.
	Slopes are then calculated and programmed into memory. 
	These values must be saved into EEPROM afterwards. 
	This function leaves the circuit off after completion.

	If the procedure fails values are not updated in memory.
*/
void calibrateCircuit(Circuit *c)
{
	int32_t regData;
	//The *Meas values are in mV or mA when read from the user
	int32_t VlowCkt,VlowMeas;
	int32_t VhighCkt,VhighMeas;
	int32_t IlowCkt,IlowMeas;
	int32_t IhighCkt,IhighMeas;
	int32_t VAhighCkt,VAhighMeas;
	int32_t VAlowCkt,VAlowMeas;
	Circuit cCal = *c;				//In case of failure so settings are not lost.


	//Clear values which need to be calibrated
	cCal.chIos = cCal.chVos = cCal.IRMSoffset = cCal.VRMSoffset = 0;
	cCal.VAoffset = cCal.Woffset = 0;
	cCal.IRMSslope = cCal.VRMSslope = cCal.VAEslope = cCal.Wslope = 1;

    RCreset();
	Cprogram(&cCal);
	ifnsuccess(_retCode) {
		dbg.println("Clearing failed in calibrateCircuit.");
		return;
	}

	CSselectDevice(cCal.circuitID);
	
	//Calibrate low level channel offsets current channel is not needed 
    //b/c the HPF is enabled (default)
	dbg.print("Ground both input lines on circuit \'");
	dbg.print(cCal.circuitID,DEC); 	dbg.println("\'."); dbg.print(PRESSENTERSTR);
	while (dbg.read() != '\r');


	//Set waveform mode to read voltage
	dbg.println("Configuring to read raw voltage.");
	ADEsetModeBit(WAVESEL_0,true); ifnsuccess(_retCode) return;
	ADEsetModeBit(WAVESEL1_,true); ifnsuccess(_retCode) return;
	ADEsetIrqEnBit(WSMP,true);	//The WAVEFORM register will not work without this.
	ADEsetIrqEnBit(CYCEND,true);/*Just in case */ ifnsuccess(_retCode) return;

	CsetOn(&cCal,true);
    delay(1000);

	//Read waveform and set CH2OS (voltage) +500mV/10322/LSB in WAVEFORM
	dbg.println("Setting voltage offset.");
	ADEgetRegister(RSTSTATUS,&regData); //reset interrupt
	ADEwaitForInterrupt(WSMP,waitTime);
	ifnsuccess(_retCode) {CsetOn(&cCal,false); dbg.println("Waiting for WSMP failed."); return;}
	ADEgetRegister(WAVEFORM,&regData);
	ifnsuccess(_retCode) {dbg.println("get WAVEFORM failed"); return;}
	dbg.print("CHVwaveform:"); dbg.println(regData);
	//regData = regData*500*100/10322/161; //(1.61mV/LSB in CH2OS) and 500/10322 in WAVEFORM
	//regData = (regData*31549)>>20;  
	//The CHXOS maxes out at 2^5 as it is a 6 bit signed magnitude number
    // Waveform is a twos complement number setCHXOS converts it to the correct signed magnitude number
	if (regData > 31){
		regData = 31;
	} else if (regData < -31) {
		regData= -31;
	}
	int8_t offset = (int8_t)regData;
	cCal.chVos = offset;
	ADEsetCHXOS(2,&(cCal.chIint),&offset);
	ifnsuccess(_retCode) {dbg.println("set CHXOS 2 failed.");return;}
	dbg.print("CHVoffset:"); dbg.println(offset);
	
    //Start calibration of VRMSOS and IRMSOS
    // One point is iMax/100 and another is base (expected) load
	//Query user to place load for low V,high I measurement
	dbg.print("(120VAC 50Hz) ~(.72A) on ckt \'"); // Max current for a house more so than anything else
	dbg.print(cCal.circuitID,DEC); 	dbg.println("\'."); dbg.print(PRESSENTERSTR);
	while (dbg.read() != '\r');
	dbg.println();
	if(!getPoint(cCal,&VlowMeas,&IhighMeas, &VAhighMeas, &VlowCkt, &IhighCkt, &VAhighCkt)) return; 

	//Query user to place load for high V,low I measurement
	dbg.print("(240VAC 50Hz) ~(.050A) on ckt \'"); //Was .025A is ~imax/50
	dbg.print(cCal.circuitID,DEC); 	dbg.println("\'."); dbg.print(PRESSENTERSTR);
	while (dbg.read() != '\r');
	dbg.println();
	if(!getPoint(cCal,&VhighMeas,&IlowMeas, &VAlowMeas, &VhighCkt, &IlowCkt, &VAlowCkt)) return; 

	//Compute the VRMSOS
	//From page 46 in the ADE data sheet 
	cCal.VRMSoffset = (VhighMeas*VlowCkt-VlowMeas*VhighCkt)/(VlowMeas-VhighMeas);
	if (cCal.VRMSoffset > 0x7FF) {
		int32_t leftOver = cCal.VRMSoffset-0x7FF;
		cCal.VRMSoffset = 0x7FF;
		leftOver = leftOver*500*100/161/1561400;
		dbg.print("leftOver in offset:");
		dbg.println(leftOver);
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
	cCal.VAEslope  = cCal.Wslope = ((float)(VAlowMeas-VAhighMeas))/(VAlowCkt-VAhighCkt);

	Cprint(&dbg,&cCal);
	Cprogram(&cCal);
	*c = cCal;						//Save new settings to *c
	ifnsuccess(_retCode) {
		dbg.println(ADEFAILEDSTR);
		return;
	}
	CSselectDevice(DEVDISABLE);
    CsetOn(c, false);
	dbg.println(COMPLETESTR);
}


/**
  Enters a string into the buffer. Backspaces are supported and the string end is delimited by '\r'.
  '\r' is not included in buff. The string is null terminated so the maximum input string is bSize-1.
  */
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
/** 
	Gets a float from the user.
	@returns retCode CANCELED if user cancels input, FAILURE,or SUCCESS.
  */
int8_t CLgetFloat(HardwareSerial *ser,float *f)
{
	char buff[32] = {'\0'};
    if (!CLgetString(ser,buff,sizeof(buff))) {
        return FAILURE;
    }
    if (!strcmp(buff,"cancel")) {
        return CANCELED;
    }
    if (sscanf(buff,"%f",f)) {
        return SUCCESS;
    }
    return FAILURE;
}
/** 
	Gets a long int from the user. Hex (e.g. 0xdeadbeef) is also permitted.
	@returns retCode CANCELED if user cancels input, FAILURE, or SUCCESS.
  */
int8_t CLgetInt(HardwareSerial *ser,int32_t *d)
{
    char buff[32] = {'\0'};
    if (CLgetString(ser,buff,sizeof(buff))) {
        return FAILURE;
    }
    if (buff[0] == '0' && (buff[1] == 'x' || buff[1] == 'X')){
        if (sscanf(buff,"%lx",d)) return SUCCESS;
    } else { 
        if (sscanf(buff,"%ld",d)) return SUCCESS;
    }
    if (!strcmp(buff,"cancel")) {
        return CANCELED;
    } 
    return FAILURE;
}


/**
 * n is the number of values used in the arithmetic mean.
 * sample returns the value to be averaged. 
 * It is responsible for setting return codes. 
 * If sample sets an unseccessful return code the routine 
 * exits and returns a 0 mean and variance.
 *
 * if *var != NULL the variance is returned as well.
 *
 * Mean and var is computed using the Knuth online mean algorithm.
 * */
int32_t avg(int n, int32_t (*sample)(void*), void* context, int32_t *var) 
{
    int32_t x = 0;
    float delta = 0;
    float mean = 0;
    float m2 = 0;
    for (int i=1; i<=n; i++) {
        x = sample(context);
        ifnsuccess(_retCode) {
            if (var != NULL) *var = 0;
            return 0;
        }
        delta = x - mean;
        //Update
        mean = mean + delta/i; 
        m2 += delta*((float)x-mean);
    }
    if (var != NULL && n > 0) {
        if (n == 1) n = 2; //1 point is not big enough sample.
        *var = (int32_t)(m2/(n-1));
    }
    return (int32_t)mean;
}

