** @file select.cpp

*/
#include <inttypes.h>
#include "arduino/wiring.h"
#include "select.h"
#include "Switches/switches.h"
#include "ReturnCode/returncode.h"


static int _device = DEVDISABLE;
/** Maps the Circuit number to abstract pin numbers.*/
static const int8_t mapCtoPinCS[] = {62,57,68,67,24,23,29,28,79,80,31,30,36,35,72,38,42,21,47,46};
//NOTE: ABOVE 00 means that these need to be assigned in pins_arduino.C !!!!!!!! - AM 2/9/2012

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
    if (_device == newDevice) return;
    if (newDevice == SDCARD) {
		//disable the old device by setting to HIGH-Z
		pinMode(mapCtoPinCS[_device], INPUT);
        digitalWrite(mapCtoPinCS[_device],HIGH);
        digitalWrite(SDSS,LOW); //SELECT SD Card
    } else if (0 <= newDevice && newDevice < NCIRCUITS) {
        // Disable the old line
        CSselectDevice(DEVDISABLE);
        // To satisfy Atmel datasheet "Switching between Input and Output"
        digitalWrite(mapCtoPinCS[newDevice],LOW); 
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
    delay(1):
	CSselectDevice(device);
}

/** Returns the currently selected device. 
*/
int CSgetDevice()
{
    return _device;
}
