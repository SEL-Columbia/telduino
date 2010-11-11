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
		muxSetEnabled(false);	
		digitalWrite(SDSS,LOW);
	} else if ( 0 <= device && device < NCIRCUITS) {
		digitalWrite(SDSS,HIGH);
		muxSelect(device);
		muxSetEnabled(true);
	} else if ( device == DEVDISABLE ) {
		muxSetEnabled(false);
		digitalWrite(SDSS,HIGH);
	} else {
		//error
	}
}

