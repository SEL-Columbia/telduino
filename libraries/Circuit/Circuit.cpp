#include "ReturnCode/returncode.h"
#include "ADE7753/ADE7753.h"

#include "Circuit.h"
#include "Select/select.h"

static Circuit circuits[NCIRCUITS]; 

#define MAGIC 2014/10000

/** 
  Updates circuit measured parameters
  @TODO Achintya what is this number: MAGIC?
  @TODO how to handle case were linecycle doesn't fire?
  @warning a communications error may leave Circuit *c in an inconsisent state.
  @return ARGVALUEERR if the circuitID is invalid
  @return COMMERR if there is a communications error or the ADE is not detected
  @return FAILURE if there is no zero crossing detected 
*/
int8_t Cupdate(Circuit *c)
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
	c->status &= ~FAULT;

	//Check for pressence and clear the interrupt register
	ifnsuccess(retCode = ADEgetRegister(RSTSTATUS,&regData)) {
		c->status |= COMM;
		CSSelectDevice(DEVDISABLE);
		return retCode;
	} 
	c->status &= 0xFFFFFFFF00000000;
	c->status |= regData;
	//TODO Update SAG and safety status
	//return FAILURE if no zero crossing detected.
	if (regData & ZXTO) {
		CSSelectDevice(DEVDISABLE);
		return FAILURE;
	}


	//Start measuring
	ifnsuccess(retCode = ADEwaitForInterrupt(LINCYC)){
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
	ifnsuccess(retCode = ADEgetRegister(PERIOD,&regData)){
		if (retCode == COMMERR) {
			c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
	}
	c->frequency =(uint16_t)(10000000/(regData*22)); //2.2us/LSbit
	//c->frequency = 1000000/(regData*2200/1000); //2.2us/LSbit

	if (!timeout) {
		//Apparent power or Volt Amps
		ifnsuccess(retCode == ADEgetRegister(LVAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->VA = regData*MAGIC/(c->halfCyclesSample/c->frequency);

		//Apparent power or Volt Amps
		ifnsuccess(retCode == ADEgetRegister(LAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->W = regData*MAGIC/(c->halfCyclesSample/c->frequency);

		//Apparent energy accumulated since last query
		ifnsuccess(retCode == ADEgetRegister(RVAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->VAEnergy = regData*MAGIC;

		//Actve energy accumulated since last query
		ifnsuccess(retCode == ADEgetRegister(RAENERGY,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->AEnergy = regData*MAGIC;

		//IRMS
		ifnsuccess(retCode == ADEgetRegister(IRMS,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->IRMS = regData/IRMSlope;

		//VRMS
		ifnsuccess(retCode == ADEgetRegister(VRMS,&regData)){
			if (retCode == COMMERR) c->status |= COMM;
			CSSelectDevice(DEVDISABLE);
			return retCode;
		}
		c->VRMS= regData/VRMSlope;

		//PF
		c->PF = (uint16_t)(((uint64_t)c->AEnergy<<16)/c->VAEnergy);
	}


	CSSelectDevice(DEVDISABLE);

	if (timeout){
		return TIMEOUT;
	}
}

int8_t Cconfigure(const Circuit *c)
{

	int8_t retCode = SUCCESS;
	int32_t regData;

	ifnsuccess(retCode = CSSelectDevice(c->circuitID)) return retCode;

	regData = c->sagDurationCycles + 1;
	if (regData >= 2) { 
		ifnsuccess(retCode = ADEsetModeBit(DISSAG,true)) return retCode;
		ifnsuccess(retCode = ADEsetRegister(SAGCYC,&regData)) return retCode;
	} else {
		ifnsuccess(retCode = ADEsetModeBit(DISSAG,false)) return retCode;
	}
	regData = c->chIos;
	ifnsuccess(retCode = ADEsetCHXOS(c->chIint,&regData)) return retCode;
	regData = c->chIgainExp;
	ifnsuccess(retCode = ADEsetRegister(GAIN,&regData)) return retCode;
	regData = c->IRMSOffset;
	ifnsuccess(retCode = ADEsetRegister(IRMSOS, &regData)) return retCode;
	regData = c->chVos;
	ifnsuccess(retCode = ADEsetCHXOS(false,&regData)) return retCode;
	regData = c->VRMSffset;
	ifnsuccess(retCode = ADEsetRegister(VRMSOS, &regData)) return retCode;
	regData = c->halfCyclesSample;
	if (regData > 0) {
		ifnsuccess(retCode = ADEsetRegister(LINECYC,&regData)) return retCode;
		ifnsuccess(retCode = ADEsetModeBit(CYCEND,true)) return retCode;
	} else {
		ifnsuccess(retCode = ADEsetModeBit(CYCEND,false)) return retCode;
	}
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
