/** @file select.cpp

*/
#include <inttypes.h>
#include "arduino/wiring.h"
#include "select.h"
#include "Demux/demux.h"
#include "ReturnCode/returncode.h"

static int _device;
/**
*   Initiallizes the SDSS pin and the muxer. 
*/
void initSelect()
{
    //SD SS
    pinMode(SDSS,OUTPUT);
    initDemux();
	CSselectDevice(DEVDISABLE);
}

/** 
*   Select SPI device  using SS(CS)
*   when device == -1 SS(CS) is HIGH (disabled)
*/
void CSselectDevice(int device) 
{
    if (device == SDCARD) {
        muxSetEnabled(false);   
        digitalWrite(SDSS,LOW);
		_device = device;
    } else if ( 0 <= device && device < NCIRCUITS) {
        digitalWrite(SDSS,HIGH);
        muxSelect(device);
        muxSetEnabled(true);
		_device = device;
    } else if ( device == DEVDISABLE ) {
        muxSetEnabled(false);
        digitalWrite(SDSS,HIGH);
		_device = device;
    } else {			//error
		_retCode = ARGVALUEERR;
	}
}

/**
	Disables and restores the selected SS pin. 
	Useful if SPI communications has gone haywire.
  */
void CSstrobe() 
{
	int device = _device;
	CSselectDevice(DEVDISABLE);
	for (int i=0; i < 10; i++); //wait
	CSselectDevice(device);
}

int CSgetDevice()
{
	return _device;
}

void CSsetEnable(int8_t enabled)
{
	if (enabled) {
		CSselectDevice(_device);
	} else {
		CSselectDevice(DEVDISABLE);
	}
}
