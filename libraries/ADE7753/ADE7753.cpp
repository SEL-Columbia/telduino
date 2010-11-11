#include "ADE7753.h"

/*
void sd_raw_send_byte(uint8_t b)
{
    SPDR = b;
    // wait for byte to be shifted out 
    while(!(SPSR & (1 << SPIF)));
    SPSR &= ~(1 << SPIF);
}

uint8_t sd_raw_rec_byte()
{
    // send dummy data for receiving some 
    SPDR = 0xff;
    while(!(SPSR & (1 << SPIF)));
    SPSR &= ~(1 << SPIF);

    return SPDR;
}

*/
bool initSPI()
{
        //Change SPI speed/endianness as empirically determined. -JR    
}

bool readData(ADEReg reg, byte data[3])
{
        int8_t numBytes = (reg.nbits+7)/8;
        int8_t ii = 0;
        if (reg.addr & 0b11000000) {
                //error: not a read instruction -AM
                return false;
        }
		
        //initialize data to be all zeros first -AM
        for (ii=0; ii<3; ii++) {
            data[ii] = 0x00;
        }

	Serial2.println(static_cast<uint8_t> (data[0]),BIN);
	Serial2.println(static_cast<uint8_t> (data[1]),BIN);
	Serial2.println(static_cast<uint8_t> (data[2]),BIN);

        //now transfer the readInstuction/registerAddress: i.e. 00xxxxxx -AM
        //sd_raw_send_byte(reg.addr);
	SPI.transfer(reg.addr);
	delayMicroseconds(4);
        //now read the data on the SPI data register byte-by-byte with the MSB first - AM
        for (ii=0; ii<numBytes; ii++) {
            data[ii] = SPI.transfer(0x00);//sd_raw_rec_byte();
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
        //SPI.transfer(regAddr);

        //now write the data on the SPI data register byte-by-byte with the MSB first - AM
        for (ii=0; ii<numBytes; ii++) {
                SPI.transfer(data[ii]);
        }

        //make sure that the data buffer is properly organized -AM
        //ans: it is -AM

        return true;
}
