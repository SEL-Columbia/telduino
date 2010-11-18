#include "ADE7753.h"

bool initSPI()
{
		//Change SPI speed/endianness as empirically determined. -JR	
	//Needs to be done before reads/writes
}
/**
* returns BYTES from the ADE in a uint32_t value
* MSB of ADE output is the MSB of the data output
* @warning SPI mode is changed after calling this funciton
* *TODO : Chksum if chksum fails return 2
*/
int readData(ADEReg reg, uint32_t *data)
{
	SPI.setDataMode(SPI_MODE1);
		int nBytes = (reg.nBits+7)/8;

	*data = 0;

		//now transfer the readInstuction/registerAddress: i.e. 00xxxxxx -AM
	SPI.transfer(reg.addr);
	//delayMicroseconds(4);
		//now read the data on the SPI data register byte-by-byte with the MSB first - AM
	const int msb = sizeof(*data)-1;
		for (int i=0; i<nBytes; i++) {
			((byte*)data)[msb-i] = SPI.transfer(0x00);
		}

		return 0;
}


/**
	returns 1 if the read failed
	returns 2 if the CHKSUM fails
*/
int ADEgetRegister(ADEReg reg, int32_t *regValue)
{
	//get raw data, MSB of data is MSB from ADE irrespective of byte length
	uint32_t rawData = 0;
	int nBytes = (reg.nBits+7)/8;

	int failure = readData(reg, &rawData);
	if (failure) {
		return failure;
	}
	
	//Push bits into MSB for irregular sizes
	rawData <<= (nBytes*8-reg.nBits);
	if (reg.signType == TWOS) {
		//Make signed
		*regValue = rawData;
	} else if(reg.signType == SIGNMAG) {
		bool sign = rawData&(1<<(sizeof(uint32_t)-1));
		rawData &= ~(1<<(sizeof(uint32_t)-1));
		if (sign) { //MSB is one
			*regValue = -rawData;
		} else {
			*regValue = rawData;
		}
	} else { //unsigned
		*regValue = rawData;
	}
	//Use signed shift for 8 byte alignment, then to move LSB to 0 byte
	(*regValue) >>= (nBytes*8-reg.nBits);
	(*regValue) >>= ((sizeof(uint32_t)-nBytes)*8);

	return 0;
}

int writeData(ADEReg reg, uint32_t *data)
{
	SPI.setDataMode(SPI_MODE1);
		int nBytes = (reg.nBits+7)/8;

		//now transfer the write Instuction/registerAddress: i.e. 10xxxxxx -JR
	SPI.transfer(reg.addr | 0x80);
		//now write the data on the SPI data register byte-by-byte with the MSB first - AM
	const int msb = sizeof(*data)-1;
		for (int i=0; i<nBytes; i++) {
			 SPI.transfer( ((byte*)data)[msb-i] );
		}
	//TODO CHKSUM 

		return 0;
}

int chksum(uint32_t data) 
{
	int sum = 0;
	for (int i=0; i < sizeof(data)*8; i++) {
		sum += data & 0x01;
		data >>= 1;
	}
	return sum;
}

