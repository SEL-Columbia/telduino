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
#define	ERRCHECKRETURN(Cptr) if (_shouldReturn(Cptr)) return;

int8_t _shouldReturn(Circuit *c) 
{
	ifnsuccess(_retCode) {			
		int8_t retCode = _retCode;	
		if (_retCode == COMMERR) {		
			c->status |= COMM;			
			CSselectDevice(DEVDISABLE);	
			_retCode = COMMERR;			
			dbg.println("returning");
			return true;						
		} else if (_retCode == TIMEOUT) {
			//Do Nothing
		} else {						
			CSselectDevice(DEVDISABLE);	
			_retCode = retCode;		
			dbg.println("returning");
			return true;						
		}								
	}
	return false;
}
/** 
  Updates circuit measured parameters
  @TODO Achintya what is this number: MAGIC?
  @warning a communications error may leave Circuit *c in an inconsisent state.
  @warning The completion time of this function is dependent on the frequency of the line as well as halfCyclesSample. At worst the function will take one minute to return if halfCyclesSample is 1400 and the frequency drops below 40hz.
  @return ARGVALUEERR if the circuitID is invalid
  @return COMMERR if there is a communications error or the ADE is not detected
*/
void Cmeasure(Circuit *c)
{
	int32_t regData;
	int8_t timeout = false;
	CSselectDevice(c->circuitID);					ERRCHECKRETURN(c);

	//Check for presence and clear the interrupt register
	ADEgetRegister(RSTSTATUS,&regData);				ERRCHECKRETURN(c);
	c->status &= ~COMM;
	c->status &= 0xFFFF0000;
	c->status |= (0x0000FFFF&regData);

	//Start measuring
	ADEgetRegister(PERIOD,&regData);				ERRCHECKRETURN(c);
	c->periodus = regData*22/10;
	dbg.print("Peruiodus:");dbg.println(c->periodus,DEC);

	uint16_t waitTime = (uint16_t)((regData*22/100)*(c->halfCyclesSample/100));
	waitTime = waitTime + waitTime/2;//Wait at least 1.5 times the amount of time it takes for halfCycleSample halfCycles to occur
	dbg.print("waitTime:");dbg.println(waitTime,DEC);
	//uint16_t waitTime = 2*1000*c->halfCyclesSample/max(c->frequency,40);

	ADEwaitForInterrupt(CYCEND,waitTime);			ERRCHECKRETURN(c);
	//The failure may have occured because there was no interrupt
	if (_retCode == TIMEOUT) {
		timeout = true;
	}

	if (!timeout) {
		//Apparent power or Volt Amps
		ADEgetRegister(LVAENERGY,&regData);			ERRCHECKRETURN(c);
		c->VA = regData*MAGIC/(c->halfCyclesSample*c->periodus/1000/1000);

		//Apparent power or Volt Amps
		ADEgetRegister(LAENERGY,&regData);			ERRCHECKRETURN(c);
		c->W = regData*MAGIC/(c->halfCyclesSample*c->periodus/1000/1000);

		//Apparent energy accumulated since last query
		ADEgetRegister(RVAENERGY,&regData);			ERRCHECKRETURN(c);
		c->VAEnergy = regData*MAGIC;

		//Actve energy accumulated since last query
		ADEgetRegister(RAENERGY,&regData);			ERRCHECKRETURN(c);
		c->WEnergy = regData*MAGIC;

		//IRMS
		ADEgetRegister(IRMS,&regData);				ERRCHECKRETURN(c);
		c->IRMS = regData/c->IRMSslope;

		//VRMS
		ADEgetRegister(VRMS,&regData);				ERRCHECKRETURN(c);
		c->VRMS= regData/c->VRMSslope;

		//Power Factor PF
		if (c->VAEnergy != 0){ 
			c->PF = (uint16_t)(((((uint64_t)c->WEnergy<<16)-c->WEnergy)-c->WEnergy)/c->VAEnergy);
		} else {
			c->PF = 65535;
		}
	}//end if (!timeout)

	CSselectDevice(DEVDISABLE);

	if (timeout){
		_retCode = TIMEOUT;
	}
}

void Cprogram(Circuit *c)
{

	_retCode = SUCCESS;
	int32_t regData;
	CSselectDevice(c->circuitID);					ERRCHECKRETURN(c);

	ADEreset();

	regData = c->sagDurationCycles + 1;
	if (regData >= 2) { 
		ADEsetModeBit(DISSAG,false);				ERRCHECKRETURN(c);
		ADEsetRegister(SAGCYC,&regData);			ERRCHECKRETURN(c);
	} else {
		ADEsetModeBit(DISSAG,true);					ERRCHECKRETURN(c);
	}
	ADEsetCHXOS(1,&c->chIint,&c->chIos);			ERRCHECKRETURN(c);
	regData = c->chIgainExp;
	ADEsetRegister(GAIN,&regData);					ERRCHECKRETURN(c);
	regData = c->IRMSoffset;
	ADEsetRegister(IRMSOS, &regData);				ERRCHECKRETURN(c);
	//since this is channel 2 c->chIint is ignored
	ADEsetCHXOS(2,&c->chIint,&c->chVos);			ERRCHECKRETURN(c);
	regData = c->VRMSoffset;
	ADEsetRegister(VRMSOS, &regData);				ERRCHECKRETURN(c);
	regData = c->halfCyclesSample;
	if (regData > 0) {
		ADEsetRegister(LINECYC,&regData);			ERRCHECKRETURN(c);
		ADEsetModeBit(CYCMODE,true);				ERRCHECKRETURN(c);
	} else {
		ADEsetModeBit(CYCMODE,false);				ERRCHECKRETURN(c);
	}
	
	CSselectDevice(DEVDISABLE);
}

void CsetOn(Circuit *c, int8_t on) 
{
	SWset(c->circuitID,on);
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

/** 
	Reasonable default values for Circuit.
	@warning Does not program the ADE7753s
  */
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
