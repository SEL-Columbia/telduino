#ifndef ADE7753_H
#define ADE7753_H

#include <inttypes.h> //check dir structure for the wiring lib -AM
#include "SPI/SPI.h"

//register defines -AM
const uint8_t IRMS = 0x16;
const uint8_t VRMS = 0x17;

/*
 * this implicitly assumes that SPI.begin has already been called -AM
 */
bool initSPI(); 

/* Assumptions that are needed for readData to work:
	1. SPI library has been initalized with SPI.begin cmd.
	2. The CS line for the SPI device that we are interested in talking to 
		has been set low.
	3. Additionally it makes sense for CS to be set high immediatly after each SPI
		operation
	-AM
*/
bool readData(int8_t numBits, int8_t regAddr, uint8_t[3] data);

#endif
