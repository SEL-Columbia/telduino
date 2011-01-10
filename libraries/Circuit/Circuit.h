#ifndef CIRCUIT_H
#define CIRCUIT_H
#include <stdint.h>

static const uint32_t ENABLED = 0x000100000000;
static const uint32_t ON      = 0x000200000000;
static const uint32_t COMM	  = 0x000400000000;

typedef struct {

	/**
	  The SPI channel associated with this particular circuit.
	  For best results halfCyclesSample/frequency should be an integer.
	  */
	int8_t circuitID;
	uint8_t frequency; 
	//This is a configuration parameter as the ADE doesn't have a frequency
	//register.

	/**
	  Measured variables
	*/
	int32_t IRMS;
	int32_t VRMS;
	int32_t VA;			//Volt Amps. i.e. Apparent energy
	int32_t W;
	//Power factor a number between 0 and 2**16 where 2**16 is a PF of 1.0 
	int16_t PF;			
	int32_t VAEnergy;	//Apparent energy since last read
	int32_t AEnergy;	//Active energy since last read
	/**
	  Status indicates if a safety fault was detected
	  The first 16bits are directly from the ADE.
	  bit name
	  16 Enabled
	  17 On/Off
	  18 Fault
	  19 CommErr
	*/
	uint32_t status;

	//Configuration parameters
	/** 
	  Measurement parameters
	*/
	uint8_t linCycMode;
	uint16_t halfCyclesSample;

	/** 
	  Current parameters
	*/
	int8_t chIint;
	int8_t chIos;
	int8_t chIgainExp;
	int16_t IRMSOffset;
	int32_t IRMSSlope;	//Converts measured units into Amps

	/** 
	  Voltage parameters
	  See ADE documentation for valid ranges.
	*/
	int8_t chVos;
	int16_t VRMSOffset;
	
	int32_t VRMSSlope;	//Converts measured units into Volts

	/**
	  Power parameters
	*/
	int32_t VASlope;	//Why isn't this a slope?

	/**
	  Software Safety parameters
	*/
	int16_t sagDurationCycles;
	int16_t minVSag;

} Circuit;


/** 
  Updates circuit measured parameters
  @warning The ADE SS line should be set before calling Cupdate.
  @returns a return code
*/
int8_t Cupdate(Circuit c);
int8_t Cconfigure(Circuit c);

#endif
