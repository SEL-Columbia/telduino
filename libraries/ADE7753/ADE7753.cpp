#include "ADE7753.h"

bool initSPI()
{
        //Change SPI speed/endianness as empirically determined. -JR    
	//Needs to be done before reads/writes
}
/**
* @warning SPI mode is changed after calling this funciton
*/
uint8_t readData(ADEReg reg, uint32_t *data)
{
	SPI.setDataMode(SPI_MODE1);
        int8_t numBytes = (reg.nBits+7)/8;

	*data = 0;

        //now transfer the readInstuction/registerAddress: i.e. 00xxxxxx -AM
	SPI.transfer(reg.addr);
	//delayMicroseconds(4);
        //now read the data on the SPI data register byte-by-byte with the MSB first - AM
        for (int ii=numBytes-1; ii>=0; ii--) {
            ((byte*)data)[ii] = SPI.transfer(0x00);
        }

	//The data buffer is now in local endianness

        return 0;
}

/**
	returns 1 if the read failed
	returns 2 if the CHKSUM fails
*/
uint8_t ADEgetRegister(ADEReg reg, int32_t *regValue)
{
	//get raw data
	uint8_t failure = readData(reg, (uint32_t*)regValue);
	if (failure) {
		return 1;
	}
	
	//fix to 32 bit signed int based on sign
	//only unsigned for now
	//TODO CONVERT PROPERLY
	//TODO signed insert into higher order then shift

	//TODO: Chksum
	//if chksum fails return 2

	return 0;
}

//data 0 is the MSB coming in first from the ADE
//TODO: to support signs should load all right up against msb then shift 
//using sign
