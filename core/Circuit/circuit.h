#ifndef CIRCUIT_H
#define CIRCUIT_H
#include <stdint.h>

#include "arduino/HardwareSerial.h"

static const uint32_t COMM	= 0x00010000;
static const uint32_t TIME  = 0x00020000;

//MODEL
typedef struct {

	/**
	  circuitID is the logical SPI CS and switch associated 
      with this particular circuit.
	  */
	int8_t circuitID;
    /**
     * Indicates whether or not this circuit is connected.
    */
    int8_t connected;

	/** 
     * Determines how often the ADE updates linecycle registers 
     * such as LAENERGY and LVAENERGY.
	  @warning cyclesSample/frequency should be an integer. \
      cyclesSample must be less than 700.
	*/
	uint16_t cyclesSample;
    /**
     * ADE Phase calibration, determined mathmatically.
     * */
    int8_t phcal;

	/** 
	  ADE Current parameters
	*/
	int8_t chIint;      //Current channel integrator
	int8_t chIos;       // Channel I offset
	int8_t chIgainExp;  // Gain exponent up to 4
	int16_t IRMSoffset; // IRMS value offset
	float IRMSslope;	// Converts measured units into Amps

	/** 
	  ADE Voltage parameters
	  See ADE documentation for valid ranges.
	*/
	int8_t chVos;       // V Waveform offsert
    int8_t chVgainExp;  // ADE page 16
    int8_t chVscale;    // Voltage scale from .125 to .5
	int16_t VRMSoffset; // VRMS offset
	float VRMSslope;	// Converts measured units into Volts

	/**
	  Software Power parameters which convert counts 
      into engineering units.
	*/
	float VAslope;	
	int32_t VAoffset;	
	float Wslope;		
	int32_t Woffset;

	/**
     * Parameters which are reported in the status field
	*/
	int16_t sagDurationCycles;
	int16_t minVSag;
    int32_t VAPowerMax;
    int32_t ipeakMax;
    int32_t vpeakMax;

	/**
	  Measured variables
	*/
	int32_t IRMS;		// in mA
	int32_t VRMS;		// in mV
	int32_t periodus;	// in ms
	int32_t VA;			// Volt Amps. i.e. Apparent energy
	int32_t W;          // Watts. i.e. active energy

	/** Power factor is a number between 0 and 2**16-1, where \
     * 2**16-1 is a PF of 1.0. */
	uint16_t PF;			
	int32_t VAEnergy;	//Apparent energy since last read
	int32_t WEnergy;	//Active energy since last read
    int32_t ipeak;
    int32_t vpeak;
	/**
     *
 	 * Status indicates if a safety fault was detected
	 * The LSB 16bits are directly from the ADE.
	 * bit name
     * 0  Active energy is half-ful
     * 1  SAG occured over interval specified in sagDurationCycles
     * 2  cyclesSample number of cycles have occured
     * 3  WAVEFORM register ready to sample
     * 4  Zero crossing
     * 5  Temperature data ready
     * 6  Reset complete
     * 7  Active (AENERGY) energy register overflowed
     * 8  Peak voltage passed
     * 9  Peak current passed 
     * 10 Apparent power (VAENERGY) is half full
     * 11 Apparent power (VAENERGY) overfloweda
     * 12 Zero crossing timeout
     * 13 Power moving to negative from positive
     * 14 Power moving to positive from negative
     *
	 * 16 Enabled 
	 * 17 CommErr
	*/
	uint32_t status;


} Circuit;

//MODEL ACCESSORS
void CsetOn(Circuit *c, int8_t on);
int8_t CisOn(Circuit *c);
uint16_t CcalcWaitTime(Circuit *c);
void CwaitForZX10(int8_t waitTime);
void CsetDefaults(Circuit *c, int8_t circuitID);
void Creset(Circuit *c);
void Cprogram(Circuit *c);
void Cclear(Circuit *c);
void Cmeasure(Circuit *c);
int8_t Cstrobe(Circuit*);
int8_t CrestoreCommunications(Circuit *c);
int8_t CtestComms(Circuit *c);
int32_t Cvrms(void*);
int32_t Cirms(void*);
int32_t Cwaveform(void*);

void Cload(Circuit *c, Circuit* addrEEPROM);
void Csave(Circuit *c, Circuit* addrEEPROM);

//CONTROLLER
typedef void (CSET)(Circuit *c,float);
CSET CsetWslope;
CSET CsetVAslope;
extern char* PARAMETERS[2];
extern CSET* CSETS[];

//VIEW
void Cprint(HardwareSerial *ser, Circuit *c);
void CprintMeas(HardwareSerial *ser, Circuit *c);

#endif
