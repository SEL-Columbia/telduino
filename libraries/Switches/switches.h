#ifndef SWITCHES_H
#define SWITCHES_H

#include <inttypes.h>
#include "arduino/wiring.h"


#ifdef __cplusplus
extern "C" {
#endif

#define NSWITCHES 2

//Enabled circuits
static int8_t _enabledC[NSWITCHES] = {1};

void SWinit();
void SWsetSwitches(int8_t enabledC[NSWITCHES]);
void SWset(int8_t sw, int8_t on);
void SWallOff();
void SWallOn();
const int8_t* SWgetSwitchState();
uint8_t SWisOn(int8_t sw);

#ifdef __cplusplus
}
#endif
#endif
