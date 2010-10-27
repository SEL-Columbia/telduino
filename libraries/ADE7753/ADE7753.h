#ifndef ADE7753_H
#define ADE7753_H

#include <inttypes.h> //check dir structure for the wiring lib -AM
#include "SPI/SPI.h"

//register defines -AM
const uint8_t IRMS = 0x16;
const uint8_t VRMS = 0x17;

bool initSPI() //this implicitly assumes that SPI.begin has already been called -AM
{
	
}

/* Assumptions that are needed for readData to work:
	1. SPI library has been initalized with SPI.begin cmd.
	2. The CS line for the SPI device that we are interested in talking to 
		has been set low.
	3. Additionally it makes sense for CS to be set high immediatly after each SPI
		operation
	-AM
*/
bool readData(int8_t numBits, int8_t regAddr, uint8_t[3] data)
{
	int8_t numBytes = (numBits+7)/8;
	int8_t ii = 0;
	if (regAddr & 0b11000000) {
		//error: not a read instruction -AM
		return false;
	}
	
	//now transfer the readInstuction/registerAddress: i.e. 00xxxxxx -AM
	SPI.transfer(regAddr);
	
	//initialize data to be all zeros first -AM
	for (ii=0; ii<3; ii++) {
		uint8_t[ii] = 0x00;
	}
	
	//now read the data on the SPI data register byte-by-byte with the MSB first - AM
	for (ii=0; ii<numBytes; ii++) {
		uint8_t[ii] = SPI.transfer(0x00);
	}
	
	//make sure that the data buffer is properly organized -AM
	//ans: it is -AM
	
	
	return true;
	
}







#endif
