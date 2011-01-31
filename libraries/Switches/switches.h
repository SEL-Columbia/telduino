#ifndef SWITCHES_H
#define SWITCHES_H

#include <inttypes.h>
#include "ShiftRegister/shiftregister.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
      This mapping from shift register positions to circuit IDs is its own inverse.
	        */
static const int8_t mapRegToSw[] = {0,1,2,3,7,6,5,4,8,9,10,11,15,14,13,12,19,18,17,16,20,21,22,23};

//Enabled circuits
static int8_t _enabledC[WIDTH];

void SWsetSwitches(int8_t enabledC[WIDTH]);
void SWset(int8_t sw, int8_t on);
void SWallOff();
void SWallOn();
const int8_t* SWgetSwitchState();
uint8_t SWisOn(int8_t sw);

#ifdef __cplusplus
}
#endif
#endif
