/** @file select.cpp

*/
#include "select.h"

/**
*	Initiallizes the SDSS pin and the muxer. 
*/
void initSelect()
{
	//SD SS
	pinMode(SDSS,OUTPUT);
	initDemux();
}

/** 
*	Select SPI device  using SS(CS)
* 	when device == -1 SS(CS) is HIGH (disabled)
*/
boolean selectSPIDevice(int8_t device) 
{
	if (device == SDCARD) {
		digitalWrite(SDSS,LOW);
		muxSetEnabled(false);	
	} else if ( 0 <= device && device < NCIRCUITS) {
		digitalWrite(SDSS,HIGH);
		muxSetEnabled(true);
		muxSelect(device);
	} else if ( device == DEVDISABLE ) {
		digitalWrite(SDSS,HIGH);
		muxSetEnabled(false);
	} else {
		//error
	}
}
