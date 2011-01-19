#include <avr/eeprom.h>

#include "ReturnCode/returncode.h"
#include "ADE7753/ADE7753.h"
#include "Select/select.h"
#include "Switches/switches.h"
#include "circuit.h"

#define MAGIC 2014/10000
#define max(X,Y) ((X)>=(Y))?(X):(Y)

/** 
  Updates circuit measured parameters
  @TODO Achintya what is this number: MAGIC?
  @TODO how to handle case were linecycle doesn't fire?
  @warning a communications error may leave Circuit *c in an inconsisent state.
  @warning The completion time of this function is dependent on the frequency of the line as well as halfCyclesSample. At worst the function will take one minute to return if halfCyclesSample is 1400 and the frequency drops below 40hz.
  @return ARGVALUEERR if the circuitID is invalid
  @return COMMERR if there is a communications error or the ADE is not detected
  @return FAILURE if there is no zero crossing detected 
*/
int8_t Cmeasure(Circuit *c)
{
	int8_t retCode = SUCCESS;
	int32_t regData;
	int8_t timeout = false;
	ifnsuccess(retCode = CSSelectDevice(c->circuitID)){
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
	c->status |= regData;
	//return FAILURE if no zero crossing detected.
	if (regData & ZXTO) {
		CSSelectDevice(DEVDISABLE);
		return FAILURE;
	}

	//Start measuring
	ifnsuccess(retCode = ADEgetRegister(PERIOD,&regData)){
		if (retCode == COMMERR) {
			c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
	}
	c->periodus = regData*22/10;

	uint16_t waitTime = (uint16_t)((regData*22/100)*(c->halfCyclesSample/100));
	waitTime = waitTime + waitTime/2;//Wait at least 1.5 times the amount of time it takes for halfCycleSample halfCycles to occur
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
		c->AEnergy = regData*MAGIC;

		//IRMS
		ifnsuccess(retCode = ADEgetRegister(IRMS,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->IRMS = regData/c->IRMSSlope;

		//VRMS
		ifnsuccess(retCode = ADEgetRegister(VRMS,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->VRMS= regData/c->VRMSSlope;

		//PF
		c->PF = (uint16_t)(((uint64_t)c->AEnergy<<16)/c->VAEnergy);
	}


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
	regData = c->IRMSOffset;
	ifnsuccess(retCode = ADEsetRegister(IRMSOS, &regData)) return retCode;
	//since this is channel 2 c->chIint is ignored
	ifnsuccess(retCode = ADEsetCHXOS(2,&c->chIint,&c->chVos)) return retCode;
	regData = c->VRMSOffset;
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

int8_t Cenable(Circuit *c, int8_t enabled) 
{
	int32_t dummy;
	int8_t retCode;
	retCode = ADEgetRegister(DIEREV,&dummy);
	if (enabled && success(retCode)){ 
		c->status |= ENABLED;
	} else if (enabled) {
		c->status |= COMM;
		c->status &= ~ENABLED;
		return retCode;
	} else {
		c->status &= ~ENABLED;
	}
	return SUCCESS;
}

int8_t CsetOn(Circuit *c, int8_t on) 
{
	int8_t retCode;
	if (!(c->status & ENABLED)){
		retCode = FAILURE;
		return retCode;
	}
	retCode = SWset(c->circuitID,on);
	return retCode;
}

int8_t CisOn(Circuit *c) 
{
	return SWisOn(c->circuitID);
}

uint8_t* Cload(Circuit *c, uint8_t* addrEEPROM)
{
	eeprom_read_block(c,addrEEPROM,sizeof(Circuit));
	return addrEEPROM + sizeof(Circuit);
}
uint8_t* Csave(Circuit *c, uint8_t* addrEEPROM) 
{
	eeprom_update_block(c,addrEEPROM,sizeof(Circuit));
	return addrEEPROM + sizeof(Circuit);
}

void CsetDefaults(Circuit *c, int8_t circuitID) 
{
	c->circuitID = circuitID;
	c->chIint = true;
	c->chIos = 0;
	c->halfCyclesSample = 200;
	c->chIgainExp = 1;
	c->IRMSOffset = 0x01BC;
	c->IRMSSlope = 164;
	c->chVos = 0;
	c->VRMSOffset = 0x07FF;
	c->VRMSSlope = 4700;
	c->VASlope = 2014/10000.0;
	c->VAOffset = 0;
	c->sagDurationCycles = 10;
	c->minVSag = 100;

	c->status |= ENABLED;
}


/*
   To Achintya?
   What is the appEnergyDiv?
   */

//Configuration
//Set gain on current channel to be 2^1 as GAIN is an exponent GAIN is 1.
//Enable linecycle and lincyccount

// A circuit is On/Off
// A circuit has an internal ID used for switching
// A circuit is the interface to the ADE and switches
// A circuit has all of the parameters used to initialize it
// The LINCYC value determines the frequency of power value reads 

/* Measured Parameters of interest are:
 VRMS
 IRMS
 Apparent Power
 Active Power
 Frequency
 
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
