//The Select Library handles the CS for SPI communications
#ifndef SELECT_H
#define SELECT_H

#ifdef __cplusplus
extern "C"{
#endif

#define NCIRCUITS 2
#define SDSS  8

/** 
*	@warning Ensure SDCCARD is not within the range of [-1,NCIRCUITS].
*/
#define DEVDISABLE -1
#define SDCARD 24 //24 works as there are less than 22 circuits

void initSelect();
void CSselectDevice(int device);
void CSstrobe();
int CSgetDevice();

#ifdef __cplusplus
}
#endif
#endif
