#ifndef SELECT_H
#define SELECT_H

#include <stdtypes.h>
#include "arduino/wiring.h"
#include "SDRAW/sd_raw.h"
#include "Demux/Demux.h"

const int8_t NCIRCUITS = 21;

/** 
*	@warning Ensure SDCCARD is not within the range of [-1,NCIRCUITS].
*/
enum Devices { DEVDISABLE = -1, SDCARD = 24 }; //24 works as there are less than 22 circuits

/** 
*	Select SPI device  using SS(CS)
* 	when device == -1 SS(CS) is HIGH (disabled)
*/
boolean selectSPIDevice(int8_t device) 
{
	if (device == SDCARD) {
		select_card();
		muxSetEnabled(false);	
	} else if ( 0 <= device && device < NCIRCUITS) {
		unselect_card();
		muxSetEnabled(true);
		muxSelect(device);
	} else if ( device == DEVDISABLE ) {
		unselect_card();
		muxSetEnabled(false);
	} else {
		//error
	}
}

#endif
