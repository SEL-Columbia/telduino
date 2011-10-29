#include "switches.h"
#include "ReturnCode/returncode.h"

void SWinit() 
{
    for(int i=0; i<NSWITCHES; i++) {
        int pinOn = mapSWtoPinON[i];
        int pinOff = mapSWtoPinOFF[i];
        pinMode(pinOn,OUTPUT);
        pinMode(pinOff,OUTPUT);
    }
    SWsetSwitches(_enabledC);
}

/**
    The workhorse of the Switches library. This method along with _enabledC 
    is used to implement the actual switching.
*/
void _SWsetSwitches() 
{
    //TODO fill this in just need to set the right pins high/low
}

/**
	There are NSWITCHES switches in the circuit as defined in shiftregister.h.
	if enabledC[i] == 1 then the circuit is on and off if it is 0. Note that the relay actually turns ON when the circuit is OFF.
  */
void SWsetSwitches(int8_t enabledC[NSWITCHES]) 
{
	int8_t i =0;
	for (i =0; i < NSWITCHES; i++) {
		_enabledC[i] = enabledC[i];
	}
	_SWsetSwitches();
}
/**
  For any non-zero value of on the switch is turned on.
  0 <= sw < NSWITCHES
  */
void SWset(int8_t sw, int8_t on) 
{
	if (0 <= sw && sw < NSWITCHES) {
		_enabledC[sw] = on;
		_SWsetSwitches();
	} else {
		_retCode = ARGVALUEERR;
	}
}
/** 
	Turns all circuits off. This implies that the actual relays are turning on.
  */
void SWallOff()
{
	int8_t i = 0;
	for (i = 0; i < NSWITCHES; i++) {
		_enabledC[i] = false;
	}
	_SWsetSwitches();
}
/** 
	Turns all circuits on. This implies that the actual relays are turning off.
  */
void SWallOn()
{
	int8_t i;
	for (i = 0; i < NSWITCHES; i++) {
		_enabledC[i] = true;
	}
    _SWsetSwitches();
}
/**
	Returns an array of size NSWITCHES.	If entry 0 is 1 then switch 0 is on
  */
const int8_t* SWgetSwitchState()
{
	return _enabledC;
}

/**
	@return true if sw is on, false otherwise
  */
uint8_t SWisOn(int8_t sw)
{
	if (0<=sw && sw<NSWITCHES) {
		return _enabledC[sw] != 0;
	} else {
		_retCode = ARGVALUEERR;
		return false;
	}
}

