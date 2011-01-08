#ifndef CIRCUIT_H
#define CIRCUIT_H
#include <stdint.h>

typedef struct {

	/**
	  Measured variables
	*/
	int32_t IRMS;
	int32_t VRMS;
	int32_t VAmps;
	int32_t watts;
	int32_t PF;			//Power factor

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
	int16_t checkDurationms;
	int32_t maxV;
	int32_t maxI;
	int16_t maxIRMSLim;
	int16_t maxVoltLim;
	int16_t minVoltLim;
	int16_t maxPowerLim;

} Circuit;

//Given two 

#endif
