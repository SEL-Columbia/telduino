//The Select Module handles the CS for SPI communications
#ifndef SELECT_H
#define SELECT_H

#ifdef __cplusplus
extern "C"{
#endif

/** Must be the same value as NSWITCHES in Switches.h */
#define NCIRCUITS 10
#define SDSS  8

/**
 * A sentinel value which is invalid.
 */
#define DEVDISABLE -1
/** 
*	@warning Ensure SDCCARD is not within the range of [-1,NCIRCUITS].
*/
#define SDCARD 24 //24 works as there are less than 22 circuits

void initSelect();
void CSselectDevice(int device);
int CSgetDevice();
void _CSselectDevice(int device);
void CSreset(int device);

#ifdef __cplusplus
}
#endif
#endif
