#ifndef SELECT_H
#define SELECT_H

#include <inttypes.h>
#include "arduino/wiring.h"
#include "SDRAW/sd_raw.h"

extern "C"{
	#include "Demux/Demux.h"
}
const int8_t NCIRCUITS = 21;
const int8_t SDSS = 47;

/** 
*	@warning Ensure SDCCARD is not within the range of [-1,NCIRCUITS].
*/
enum Devices { DEVDISABLE = -1, SDCARD = 24 }; //24 works as there are less than 22 circuits

void initSelect();
boolean selectSPIDevice(int8_t device);

#endif
