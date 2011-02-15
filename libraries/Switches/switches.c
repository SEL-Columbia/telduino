#include "switches.h"
#include "ShiftRegister/shiftregister.h"
#include "ReturnCode/returncode.h"


void _SWsetSwitches() 
{
	uint8_t regBits[WIDTH];
	int8_t sreg;
	SRsetEnabled(true);
	for (sreg = 0; sreg < WIDTH; sreg++) {
		//We want the shiftregisters to have invered logic if enableC[i] == 1 -> switch i is on.
		regBits[sreg] = !_enabledC[mapRegToSw[sreg]];
	}
	SRshiftArray(regBits,WIDTH);
	SRlatch();
}

/**
	There are WIDTH switches in the circuit as defined in shiftregister.h.
	if enabledC[i] == 1 then the circuit is on and off if it is 0. Note that the relay actually turns ON when the circuit is OFF.
  */
void SWsetSwitches(int8_t enabledC[WIDTH]) 
{
	int8_t i =0;
	for (i =0; i < WIDTH; i++) {
		_enabledC[i] = enabledC[i];
	}
	_SWsetSwitches();
}
/**
  For any non-zero value of on the switch is turned on.
  0 <= sw < WIDTH
  */
void SWset(int8_t sw, int8_t on) 
{
	if (0 <= sw && sw < WIDTH) {
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
	for (i = 0; i < WIDTH; i++) {
		_enabledC[i] = false;
	}
	SRsetEnabled(true);
	_SWsetSwitches();
}
/** 
	Turns all circuits on. This implies that the actual relays are turning off.
  */
void SWallOn()
{
	int8_t i;
	for (i = 0; i < WIDTH; i++) {
		_enabledC[i] = true;
	}
	SRsetEnabled(true);
	SRclear();
	SRlatch();
}
/**
	Returns an array of size WIDTH.	If entry 0 is 1 then switch 0 is on
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
	if (0<=sw && sw<WIDTH) {
		return _enabledC[sw] != 0;
	} else {
		_retCode = ARGVALUEERR;
		return false;
	}
}

