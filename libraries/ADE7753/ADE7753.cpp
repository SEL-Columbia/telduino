#include "ADE7753.h"

bool initSPI()
{
        //Change SPI speed/endianness as empirically determined. -JR    
}

bool readData(int8_t numBits, int8_t regAddr, uint8_t data[3])
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
        for (ii=0; ii<numBytes; ii++) {
                data[ii] = 0x00;
        }
	delay(10);
        //now read the data on the SPI data register byte-by-byte with the MSB first - AM
        for (ii=0; ii<numBytes; ii++) {

                data[ii] = SPI.transfer(0x00);
		delay(10);
        }

        //make sure that the data buffer is properly organized -AM
        //ans: it is -AM

        return true;
}

bool writeData(int8_t numBits, int8_t regAddr, uint8_t data[3])
{
        int8_t numBytes = (numBits+7)/8;
        int8_t ii = 0;

        if (regAddr & 0b11000000) {
                //error: not a pure address -AM
                return false;
        }
        regAddr |= 0b10000000;

        //now transfer the readInstuction/registerAddress: i.e. 00xxxxxx -AM
        SPI.transfer(regAddr);

        //now write the data on the SPI data register byte-by-byte with the MSB first - AM
        for (ii=0; ii<numBytes; ii++) {
                SPI.transfer(data[ii]);
        }

        //make sure that the data buffer is properly organized -AM
        //ans: it is -AM

        return true;
}
