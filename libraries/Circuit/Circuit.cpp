#include "Circuit.h"

/*
   Achintya?
   What is the appEnergyDiv?
   */

class Circuit 
{
private:
	//Calibration parameters
	uint32_t iRMSSlope;
	uint32_t vRMSSlope;
	int16_t  irmsOS; //Maximum of 12 bits
	int16_t  vrmsOS; //Maximum of 12 bits
public:

}
//Configuration
//Set GAIN to be 2
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
 LINCYC Cycle half counts before interrupt (should be one or two times the frequncy so that a measurement occurs after 1 second)
 MODE Register:
 CYCMODE 1
 

 ADE Calibration Parameters:
 CH1OS 
 CH2OS
 VRMSOS
 IRMSOS
 LVAENERGY used to get line cycle accumulation energy or energy over a certain number of line cycles
 
 Circuit measurement calibration parameters:
 IRMS slope to turn ADE measurements into Amperes
 VRMS slope to turn ADE measurements into Volts
 Energy and power slope?

*/

//Returns an error if timeout reached when checking interrupts
//Have fault detection code check interrupts for sag detection and frequency variation
