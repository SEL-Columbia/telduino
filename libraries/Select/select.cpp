/** @file select.cpp

*/
#include "select.h"

/**
*   Initiallizes the SDSS pin and the muxer. 
*/
void initSelect()
{
    //SD SS
    pinMode(SDSS,OUTPUT);
    initDemux();
}

/** 
*   Select SPI device  using SS(CS)
*   when device == -1 SS(CS) is HIGH (disabled)
*/
int CSSelectDevice(int device) 
{
    if (device == SDCARD) {
        muxSetEnabled(false);   
        digitalWrite(SDSS,LOW);
        return 0;
    } else if ( 0 <= device && device < NCIRCUITS) {
        digitalWrite(SDSS,HIGH);
        muxSelect(device);
        muxSetEnabled(true);
        return 0;
    } else if ( device == DEVDISABLE ) {
        muxSetEnabled(false);
        digitalWrite(SDSS,HIGH);
        return 0;
    }
    //error
    return 1;
}

