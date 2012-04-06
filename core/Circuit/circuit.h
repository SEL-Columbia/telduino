#ifndef CIRCUIT_H
#define CIRCUIT_H
#include <stdint.h>

#include "arduino/HardwareSerial.h"

static const uint32_t COMM	= 0x00010000;
static const uint32_t TIME  = 0x00020000;

typedef struct {

	//Configuration parameters
	/**
	  circuitID is the SPI channel and switch associated with this particular circuit.
	  @warning halfCyclesSample/frequency should be an integer. HalfCyclesSample must be less than 1400.
	  */
	int8_t circuitID;

	/** 
	  Measurement parameters
	*/
	uint16_t halfCyclesSample;
    int8_t phcal;
    int8_t connected;

	/** 
	  Current parameters
	*/
	int8_t chIint;
	int8_t chIos;       // Channel I offset
	int8_t chIgainExp;
	int16_t IRMSoffset; // IRMS value offset
	float IRMSslope;	// Converts measured units into Amps

	/** 
	  Voltage parameters
	  See ADE documentation for valid ranges.
	*/
	int8_t chVos;
    int8_t chVgainExp; // ADE page 16
    int8_t chVscale;
	int16_t VRMSoffset;
	float VRMSslope;	// Converts measured units into Volts

	/**
	  Power parameters
	*/
	float VAEslope;	
	int32_t VAoffset;	
	float Wslope;		
	int32_t Woffset;

	/**
	  Software Safety parameters
	*/
	int16_t sagDurationCycles;
	int16_t minVSag;
    int32_t VAPowerMax;
    int32_t ipeakMax;
    int32_t vpeakMax;

	/**
	  Measured variables
	*/
	int32_t IRMS;		//in mA
	int32_t VRMS;		//in mV
	int32_t periodus;	
	int32_t VA;			//Volt Amps. i.e. Apparent energy
	int32_t W;
	//Power factor is a number between 0 and 2**16-1, where 2**16-1 is a PF of 1.0.
	uint16_t PF;			
	int32_t VAEnergy;	//Apparent energy since last read
	int32_t WEnergy;	//Active energy since last read
    int32_t ipeak;
    int32_t vpeak;
	/**
	  Status indicates if a safety fault was detected
	  The first 16bits are directly from the ADE.
	  bit name
	  16 Enabled
	  17 CommErr
	*/
	uint32_t status;


} Circuit;


/** 
  Updates circuit measured parameters
  @returns a return code
*/
uint16_t CwaitTime(Circuit *c);
void CsetOn(Circuit *c, int8_t on);
int8_t CisOn(Circuit *c);
void Cload(Circuit *c, Circuit* addrEEPROM);
void Csave(Circuit *c, Circuit* addrEEPROM);
void CsetDefaults(Circuit *c, int8_t circuitID);
void Cclear(Circuit *c);
void Cmeasure(Circuit *c);
void Cprogram(Circuit *c);
void Cprint(HardwareSerial *ser, Circuit *c);
void CprintMeas(HardwareSerial *ser, Circuit *c);
int8_t Cstrobe(Circuit*);
int8_t CrestoreCommunications(Circuit *c);
int8_t CtestComms(Circuit *c);
void Creset(Circuit *c);
void CwaitForZX10(int8_t waitTime);
int32_t Cvrms(void*);
int32_t Cirms(void*);
int32_t Cwaveform(void*);

#endif
