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

const int NCIRCUITS = 21;
const int SDSS = 47;

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
