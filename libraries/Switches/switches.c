#include "switches.h"
#include "ShiftRegister/shiftregister.h"
#include "ReturnCode/returncode.h"

/**
	There are WIDTH switches in the circuit as defined in shiftregister.h.
	if enabledC[i] == 1 then the switch is on and off if it is 0.
  */
void SWsetSwitches(int8_t enabledC[WIDTH]) 
{
	SRsetEnabled(true);
	uint8_t regBits[WIDTH];
	int8_t sreg;
	for (sreg = 0; sreg < WIDTH; sreg++) {
		regBits[sreg] = !enabledC[mapRegToSw[sreg]];
	}
	SRshiftArray(regBits,WIDTH);
	SRlatch();
}
/**
  0 <= sw < WIDTH
  */
int8_t SWset(int8_t sw, int8_t on) 
{
	if (0 <= sw && sw <= WIDTH) {
		_enabledC[sw] = on;
		SWsetSwitches(_enabledC);
		return SUCCESS;
	}
	return ARGVALUEERR;
}
void SWallOff()
{
	//The shift register code inverts the logic so that a 0 implies
	//the circuit is off.
	int8_t i = 0;
	for (i = 0; i < WIDTH; i++) {
		_enabledC[i] = 0;
	}
	SRsetEnabled(true);
	SRshiftArray(_enabledC,WIDTH);
	SRlatch();
}
void SWallOn()
{
	//The switches are always on by default so clearing the register turns
	//them all on
	int8_t i;
	for (i = 0; i < WIDTH; i++) {
		_enabledC[i] = 1;
	}
	SRsetEnabled(true);
	SRclear();
	SRlatch();
}
/**
	Returns an array of size WIDTH
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
	if ( 0<=sw && sw < WIDTH ) {
		return _enabledC[sw] == 1;
	}
}

