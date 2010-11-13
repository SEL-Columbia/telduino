#ifndef SELECT_H
#define SELECT_H

#include <inttypes.h>
#include "arduino/wiring.h"
#include "SDRaw/sd_raw.h"

extern "C"{
	#include "Demux/Demux.h"
}
const int NCIRCUITS = 21;
const int SDSS = 47;
const int muxMap[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; 

/** 
*	@warning Ensure SDCCARD is not within the range of [-1,NCIRCUITS].
*/
enum Devices { DEVDISABLE = -1, SDCARD = 24 }; //24 works as there are less than 22 circuits

void initSelect();
int CSSelectDevice(int device);

#endif
