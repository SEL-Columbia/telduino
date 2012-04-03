/** @file select.cpp
 *
 * TODO disable should be a separate function not a special device number.

*/
#include <inttypes.h>
#include "arduino/wiring.h"
#include "select.h"
#include "Switches/switches.h"
#include "ReturnCode/returncode.h"


static int _device = DEVDISABLE;
/** Maps the Circuit number to abstract pin numbers.*/
static const int8_t mapCtoPinCS[] = {62,57,68,67,24,23,29,28,79,80,31,30,36,35,72,38,42,21,47,46};

/**
 * For the ADE pins.
 * */
void CSpinActive(int8_t active,int8_t device) 
{
    if (active) {
        // Instead of setting the pins as low impedenece high outputs, let's set them as High-Z
        // In this setup adding another Port A causes communications to fail. In the other configurations it is Port B
        // To satisfy Atmel datasheet "Switching between Input and Output" in the High-Z mode add the below line
        digitalWrite(mapCtoPinCS[device],LOW); 

        pinMode(mapCtoPinCS[device],OUTPUT);
        digitalWrite(mapCtoPinCS[device],LOW);
    } else {
        //pinMode(mapCtoPinCS[device],OUTPUT);
        //digitalWrite(mapCtoPinCS[device],HIGH);
        //Instead of setting the pins as low impedenece high outputs, let's set them as High-Z
        //digitalWrite(mapCtoPinCS[device],HIGH);
        pinMode(mapCtoPinCS[device],INPUT);
        digitalWrite(mapCtoPinCS[device],HIGH);
    }
}

/**
*   Initiallizes the SDSS pin and sets all other CS pins to high.
*	All the channels are disable from communication. 
*/
void initSelect()
{
    //SD SS
    pinMode(SDSS,OUTPUT);
    digitalWrite(SDSS,HIGH); 
    /** Note that sd_raw_config has more configuration information for these pins*/
    for (int8_t i=0; i < NCIRCUITS; i++) {
        CSpinActive(false,i);
    }
    CSselectDevice(DEVDISABLE);
}

/** 
*   Select SPI device  using SS(CS)
*   when device == -1 SS(CS) is HIGH (disabled)
*	Note: ALL other devices should be held HIGH
*	and the device desired should be driven LOW
*
*	@note resets the _retCode return value
*/
void CSselectDevice(int newDevice) 
{
    if (_device == newDevice) return;
    _CSselectDevice(newDevice);
    ifsuccess(_retCode) {
        _device = newDevice;
    }
}


/**
 * Implementation of select device without error handling outside of range.
 */
void _CSselectDevice(int newDevice) {
    if (newDevice == SDCARD) {
		//disable the old device by setting to HIGH-Z
        CSpinActive(false,_device);
        digitalWrite(SDSS,LOW); //SELECT SD Card
    } else if (0 <= newDevice && newDevice < NCIRCUITS) {
        // Disable the old line
        _CSselectDevice(DEVDISABLE);
        CSpinActive(true,newDevice);
    } else if (newDevice == DEVDISABLE) {
        digitalWrite(SDSS,HIGH);
        CSpinActive(false,_device);
    } else { //error
        _retCode = ARGVALUEERR;
    }
}

/**
 * Resets both floor(device/2) and its successor
 * */
void CSreset(int device) 
{
    if (!(0 <= device && device < NCIRCUITS)) {
        _retCode = ARGVALUEERR;
        return;
    }
    CSselectDevice(DEVDISABLE);
    device = (device/2)*2;
    CSpinActive(true,device);
    CSpinActive(true,device+1);

    delay(10);
    CSpinActive(false,device);
    CSpinActive(false,device+1);
}

/** Returns the currently selected device. 
*/
int CSgetDevice()
{
    return _device;
}
