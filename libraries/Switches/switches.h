#ifndef SWITCHES_H
#define SWITCHES_H

#include <inttypes.h>
#include "arduino/wiring.h"


#ifdef __cplusplus
extern "C" {
#endif

#define NSWITCHES 2

//Enabled circuits
static int8_t _enabledC[NSWITCHES] = {0};

void SWinit(); // initialize the switching lib and set all to _enabledC
//void SWsetSwitches(int8_t enabledC[NSWITCHES]); 
//removed above becuase of lack of shift register -AM

void SWset(int8_t sw, int8_t on); //turn on or off an indv switch
void SWallOff(); // turn all off
void SWallOn(); // turn all on
const int8_t* SWgetSwitchState(); // return an array of switch states
uint8_t SWisOn(int8_t sw); // return the state of an individual switch

#ifdef __cplusplus
}
#endif
#endif
