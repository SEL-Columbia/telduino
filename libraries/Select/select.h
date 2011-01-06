#ifndef SELECT_H
#define SELECT_H

#include <inttypes.h>
#include "arduino/wiring.h"
#include "Demux/Demux.h"

#ifdef __cplusplus
extern "C"{
#endif


/**
    The muxers have an identity mapping between input value and circuit number.
  */
#define NCIRCUITS  21
#define SDSS  47
//int muxMap[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; 

/** 
*	@warning Ensure SDCCARD is not within the range of [-1,NCIRCUITS].
*/
#define DEVDISABLE -1
#define SDCARD 24 //24 works as there are less than 22 circuits

void initSelect();
int CSSelectDevice(int device);

#ifdef __cplusplus
}
#endif
#endif
