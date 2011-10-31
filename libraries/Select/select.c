/** @file select.cpp

*/
#include <inttypes.h>
#include "arduino/wiring.h"
#include "select.h"
#include "Switches/switches.h"
#include "ReturnCode/returncode.h"

static int _device;
static const int8_t mapCtoPinCS[] = {62,57};

/**
*   Initiallizes the SDSS pin and the muxer. 
*/
void initSelect()
{
    //SD SS
    pinMode(SDSS,OUTPUT);
    digitalWrite(SDSS,HIGH); 
    for (int8_t i=0; i < NCIRCUITS; i++) {
        pinMode(mapCtoPinCS[i],OUTPUT);
        //Inverted logic twice as CS LOW means active
        digitalWrite(mapCtoPinCS[i],LOW); 
    }
    CSselectDevice(DEVDISABLE);
}

/** 
*   Select SPI device  using SS(CS)
*   when device == -1 SS(CS) is HIGH (disabled)
*/
void CSselectDevice(int newDevice) 
{
    if (newDevice == SDCARD) {
        digitalWrite(mapCtoPinCS[_device],LOW);
        digitalWrite(SDSS,LOW);
    } else if ( 0 <= newDevice && newDevice < NCIRCUITS) {
        digitalWrite(SDSS,HIGH);
        digitalWrite(mapCtoPinCS[newDevice],HIGH);
    } else if ( newDevice == DEVDISABLE ) {
        digitalWrite(SDSS,HIGH);
        digitalWrite(mapCtoPinCS[_device],LOW);
    } else { //error
        _retCode = ARGVALUEERR;
    }
    _device = newDevice;
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

/** Returns the currently selected device. 
*/
int CSgetDevice()
{
    return _device;
}
