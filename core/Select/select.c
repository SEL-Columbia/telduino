/** @file select.cpp

*/
#include <inttypes.h>
#include "arduino/wiring.h"
#include "select.h"
#include "Switches/switches.h"
#include "ReturnCode/returncode.h"


// I know that static auto-initializes to 0, but why isn't it explictly declared? -AM
static int _device;
static const int8_t mapCtoPinCS[] = {62,57};

/**
*   Initiallizes the SDSS pin and sets all other CS pins to high.
*	All the channels are disable from communication. 
*/
void initSelect()
{
    //SD SS
    pinMode(SDSS,OUTPUT);
    digitalWrite(SDSS,HIGH); 
    for (int8_t i=0; i < NCIRCUITS; i++) {
	//Instead of setting the pins as low impedenece high outputs, let's set them as High-Z
//        pinMode(mapCtoPinCS[i],OUTPUT);
//        digitalWrite(mapCtoPinCS[i],HIGH); 
        pinMode(mapCtoPinCS[i],INPUT);
        digitalWrite(mapCtoPinCS[i],HIGH); 
    }
    CSselectDevice(DEVDISABLE);
}

/** 
*   Select SPI device  using SS(CS)
*   when device == -1 SS(CS) is HIGH (disabled)
*	Note: ALL other devices should be held HIGH
*	and the device desired should be driven LOW
*/
void CSselectDevice(int newDevice) 
{
    if (newDevice == SDCARD) {
		//disable the old device by setting to HIGH-Z
		pinMode(mapCtoPinCS[_device], INPUT);
        digitalWrite(mapCtoPinCS[_device],HIGH);
        digitalWrite(SDSS,LOW); //SELECT sdss
    } else if ( 0 <= newDevice && newDevice < NCIRCUITS) {
		digitalWrite(SDSS,HIGH);
		pinMode(mapCtoPinCS[newDevice], OUTPUT);
        digitalWrite(mapCtoPinCS[newDevice],LOW);
    } else if ( newDevice == DEVDISABLE ) {
        digitalWrite(SDSS,HIGH);
		pinMode(mapCtoPinCS[newDevice], INPUT);
        digitalWrite(mapCtoPinCS[_device],HIGH);
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
