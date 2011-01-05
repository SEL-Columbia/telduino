#include "switches.h"
#include "ShiftRegister/ShiftRegister.h"
#include "ReturnCode/returncode.h"

void SWsetSwitches(uint8_t enabledC[WIDTH]) 
{
	setEnabled(true);
	uint8_t regBits[WIDTH];
	int8_t sreg;
	for (sreg = 0; sreg < WIDTH; sreg++) {
		regBits[sreg] = !enabledC[mapRegToSw[sreg]];
	}
	shiftArray(regBits,WIDTH);
	latch();
}
void SWallOff()
{
	//Since the switches are always on we have to put a 1 to turn off the circuit
	//bits must be of length WIDTH
	uint8_t bits[] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
	setEnabled(true);
	shiftArray(bits,WIDTH);
	latch();
}
void SWallOn()
{
	//Since the switches are always on we have to put a 0 to turn on the circuit
	setEnabled(true);
	clearShiftRegister();
	latch();
}
