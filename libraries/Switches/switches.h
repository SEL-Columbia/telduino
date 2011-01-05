#ifndef SWITCHES_H
#define SWITCHES_H

#include "ReturnCode/returncode.h"
#include "ShiftRegister/ShiftRegister.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  This mapping from shift register positions to circuit IDs is its own inverse.
  */
const int mapRegToSw[] = {0,1,2,3,7,6,5,4,8,9,10,11,15,14,13,12,19,18,17,16,20,21,22,23};

void SWsetSwitches(uint8_t enabledC[WIDTH]);
void SWallOff();
void SWallOn();

#ifdef __cplusplus
}
#endif
#endif
