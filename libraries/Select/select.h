#ifndef SELECT_H
#define SELECT_H

#ifdef __cplusplus
extern "C"{
#endif


/**
    The muxers have an identity mapping between input value and circuit number.
  */
#define SDSS  47
//int muxMap[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; 

/** 
*	@warning Ensure SDCCARD is not within the range of [-1,NCIRCUITS].
*/
#define DEVDISABLE -1
#define SDCARD 24 //24 works as there are less than 22 circuits

void initSelect();
void CSselectDevice(int device);
void CSstrobe();
int CSgetDevice();
void CSsetEnable(int8_t enabled);

#ifdef __cplusplus
}
#endif
#endif
