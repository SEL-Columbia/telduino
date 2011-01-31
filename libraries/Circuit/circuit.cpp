#include <avr/eeprom.h>

#include "ReturnCode/returncode.h"
#include "ADE7753/ADE7753.h"
#include "Select/select.h"
#include "Switches/switches.h"
#include "circuit.h"
#include "arduino/HardwareSerial.h"


#define dbg Serial1
#define MAGIC 2014/10000
#define max(X,Y) ((X)>=(Y))?(X):(Y)

/** 
  Updates circuit measured parameters
  @TODO Achintya what is this number: MAGIC?
  @warning a communications error may leave Circuit *c in an inconsisent state.
  @warning The completion time of this function is dependent on the frequency of the line as well as halfCyclesSample. At worst the function will take one minute to return if halfCyclesSample is 1400 and the frequency drops below 40hz.
  @return ARGVALUEERR if the circuitID is invalid
  @return COMMERR if there is a communications error or the ADE is not detected
*/
int8_t Cmeasure(Circuit *c)
{
	int8_t retCode = SUCCESS;
	int32_t regData;
	int8_t timeout = false;
	ifnsuccess(retCode=CSSelectDevice(c->circuitID)){
		c->status |= COMM;
		CSSelectDevice(DEVDISABLE);
		return retCode;
	} 
	c->status &= ~COMM;

	//Check for presence and clear the interrupt register
	ifnsuccess(retCode = ADEgetRegister(RSTSTATUS,&regData)) {
		c->status |= COMM;
		CSSelectDevice(DEVDISABLE);
		return retCode;
	} 
	c->status &= 0xFFFF0000;
	c->status |= (0x0000FFFF&regData);

	//Start measuring
	ifnsuccess(retCode = ADEgetRegister(PERIOD,&regData)){
		if (retCode == COMMERR) {
			c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
	}
	c->periodus = regData*22/10;
	dbg.print("Peruiodus:");dbg.println(c->periodus,DEC);

	uint16_t waitTime = (uint16_t)((regData*22/100)*(c->halfCyclesSample/100));
	waitTime = waitTime + waitTime/2;//Wait at least 1.5 times the amount of time it takes for halfCycleSample halfCycles to occur
	dbg.print("waitTime:");dbg.println(waitTime,DEC);
	//uint16_t waitTime = 2*1000*c->halfCyclesSample/max(c->frequency,40);

	ifnsuccess(retCode = ADEwaitForInterrupt(CYCEND,waitTime)){
		if (retCode == COMMERR) {
			c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		//The failure may have occured because there was no interrupt
		if (retCode == TIMEOUT) {
			timeout = true;
		}
	}

	if (!timeout) {
		//Apparent power or Volt Amps
		ifnsuccess(retCode = ADEgetRegister(LVAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->VA = regData*MAGIC/(c->halfCyclesSample*c->periodus/1000/1000);

		//Apparent power or Volt Amps
		ifnsuccess(retCode = ADEgetRegister(LAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->W = regData*MAGIC/(c->halfCyclesSample*c->periodus/1000/1000);

		//Apparent energy accumulated since last query
		ifnsuccess(retCode = ADEgetRegister(RVAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->VAEnergy = regData*MAGIC;

		//Actve energy accumulated since last query
		ifnsuccess(retCode = ADEgetRegister(RAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->WEnergy = regData*MAGIC;

		//IRMS
		ifnsuccess(retCode = ADEgetRegister(IRMS,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->IRMS = regData/c->IRMSslope;

		//VRMS
		ifnsuccess(retCode = ADEgetRegister(VRMS,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->VRMS= regData/c->VRMSslope;

		//Power Factor PF
		if (c->VAEnergy != 0){ 
			c->PF = (uint16_t)(((((uint64_t)c->WEnergy<<16)-c->WEnergy)-c->WEnergy)/c->VAEnergy);
		} else {
			c->PF = 65535;
		}
	}//end if (!timeout)

	CSSelectDevice(DEVDISABLE);

	if (timeout){
		return TIMEOUT;
	}
}

int8_t Cprogram(const Circuit *c)
{

	int8_t retCode = SUCCESS;
	int32_t regData;

	ifnsuccess(retCode = CSSelectDevice(c->circuitID)) return retCode;

	ADEreset();

	regData = c->sagDurationCycles + 1;
	if (regData >= 2) { 
		ifnsuccess(retCode = ADEsetModeBit(DISSAG,false)) return retCode;
		ifnsuccess(retCode = ADEsetRegister(SAGCYC,&regData)) return retCode;
	} else {
		ifnsuccess(retCode = ADEsetModeBit(DISSAG,true)) return retCode;
	}
	ifnsuccess(retCode = ADEsetCHXOS(1,&c->chIint,&c->chIos)) return retCode;
	regData = c->chIgainExp;
	ifnsuccess(retCode = ADEsetRegister(GAIN,&regData)) return retCode;
	regData = c->IRMSoffset;
	ifnsuccess(retCode = ADEsetRegister(IRMSOS, &regData)) return retCode;
	//since this is channel 2 c->chIint is ignored
	ifnsuccess(retCode = ADEsetCHXOS(2,&c->chIint,&c->chVos)) return retCode;
	regData = c->VRMSoffset;
	ifnsuccess(retCode = ADEsetRegister(VRMSOS, &regData)) return retCode;
	regData = c->halfCyclesSample;
	if (regData > 0) {
		ifnsuccess(retCode = ADEsetRegister(LINECYC,&regData)) return retCode;
		ifnsuccess(retCode = ADEsetModeBit(CYCMODE,true)) return retCode;
	} else {
		ifnsuccess(retCode = ADEsetModeBit(CYCMODE,false)) return retCode;
	}
	
	CSSelectDevice(DEVDISABLE);
	return retCode;
}

int8_t CsetOn(Circuit *c, int8_t on) 
{
	int8_t retCode;
	retCode = SWset(c->circuitID,on);
	return retCode;
}

int8_t CisOn(Circuit *c) 
{
	return SWisOn(c->circuitID);
}

void Cload(Circuit *c, uint8_t* addrEEPROM)
{
	eeprom_read_block(c,addrEEPROM,sizeof(Circuit));
}
void Csave(Circuit *c, uint8_t* addrEEPROM) 
{
	eeprom_update_block(c,addrEEPROM,sizeof(Circuit));
}

void CsetDefaults(Circuit *c, int8_t circuitID) 
{
	c->circuitID = circuitID;
	c->halfCyclesSample = 200;
	c->chIint = true;
	c->chIos = 0;
	c->chIgainExp = 1;
	c->IRMSoffset = 0x01BC;
	c->IRMSslope = 164;
	c->chVos = 0;
	c->VRMSoffset = 0x07FF;
	c->VRMSslope = 4700;
	c->VAslope = 2014/10000.0;
	c->VAoffset = 0;
	c->sagDurationCycles = 10;
	c->minVSag = 100;

	//Meausured
	c->IRMS = 0;
	c->VRMS = 0;
	c->VAEnergy = 0;
	c->WEnergy = 0;
}

void CprintMeas(HardwareSerial *ser, Circuit *c)
{
	ser->print("IRMS&");
	ser->print(c->IRMS);
	ser->print(" ");
	ser->print("VRMS&");
	ser->println(c->VRMS);
}

/*
   To Achintya?
   What is the appEnergyDiv?
   */

//Configuration
//Set gain on current channel to be 2^1 as GAIN is an exponent GAIN is 1.
//Enable linecycle and lincyccount

// The LINCYC value determines the frequency of power value reads 

/* Measured Parameters of interest are:
 VRMS
 IRMS
 Apparent Power
 Active Power
 Period
 
 Derived or set parameters:
 Switch Status
 Power Factor (W/VA)
 Safety Current limit
 Safety Voltage limit

 ADE Setup Parameters:
 CH1OS Integrator switch
 GAIN for channel 2
 LINCYC Cycle half counts before interrupt (should be one or two times the 
	frequncy so that a measurement occurs after at least 1 second)
 MODE Register:
 CYCMODE 1
 

 ADE Calibration Parameters:
 CH1OS 
 CH2OS
 VRMSOS
 IRMSOS
 LVAENERGY used to get line cycle accumulation energy or energy over a 
	certain number of line cycles
 
 Circuit measurement calibration parameters:
 IRMS slope to turn ADE measurements into Amperes
 VRMS slope to turn ADE measurements into Volts
 Energy and power slope?
*/

//Have fault detection code check interrupts for sag detection and frequency variation
