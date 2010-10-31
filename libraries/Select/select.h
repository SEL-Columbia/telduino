#ifndef SELECT_H
#define SELECT_H

#include <stdtypes.h>
#include "arduino/wiring.h"
#include "SDRAW/sd_raw.h"
#include "Demux/Demux.h"

//TODO: Magic number of circuits
//Mapping between ckt as index and shift register slot as value. i.e. srMapping[0] is the bit in the ShiftRegister that corresponds to circuit 0.
const int8_t srMapping[21]; 
//Muxes map normally. i.e. mux entry 0 is the 0th circuit.
const int8_t SDCARD = 24; As there is not 32th circuit this is a safe value, for now.

//disable

//select device
boolean selectSPIDevice(int8_t device) {
	if (device == SDCARD) {
		select_card();
		muxSetEnabled(false);	
	} else if ( 0 <= device && device <= 20) {
		unselect_card();
		muxSetEnabled(true);
		muxSelect(device);
	} else {
		//error
	}

}

#endif
