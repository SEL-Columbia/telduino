#include "ADE7753.h"
#include <limits.h>
#include "arduino/WProgram.h"
//TODO Add range checks for all safe functions
//TODO writeData shouldn't use pointer neither should readData

/**
* returns BYTES from the ADE in a uint32_t value
* MSB of ADE output is the MSB of the data output
* @warning SPI mode is changed after calling this funciton
*/
void ADEreadData(ADEReg reg, uint32_t *data)
{
    uint8_t nBytes = (reg.nBits+7)/8;

    //SPI speed is f_osc/128
    SPI.setClockDivider(SPI_CLOCK_DIV128);
	SPI.setDataMode(SPI_MODE1);

	//now transfer the readInstuction/registerAddress: i.e. 00xxxxxx -AM
	SPI.transfer(reg.addr);
	//now read the data on the SPI data register byte-by-byte with the MSB first - AM
	uint8_t msb = sizeof(*data)-1;
	*data = 0;
	for (int i=0; i<nBytes; i++) {
		((byte*)data)[msb-i] = SPI.transfer(0x00);
	}
}

void ADEwriteData(ADEReg reg, uint32_t *data)
{
	uint8_t nBytes = (reg.nBits+7)/8;

    //SPI speed is f_osc/128
	SPI.setDataMode(SPI_MODE1);
    SPI.setClockDivider(SPI_CLOCK_DIV128);

	//now transfer the write Instuction/registerAddress: i.e. 10xxxxxx -JR
	SPI.transfer(reg.addr | 0x80);
	//now write the data on the SPI data register byte-by-byte with the MSB first - AM
	const uint8_t msb = sizeof(*data)-1;
	for (uint8_t i=0; i<nBytes; i++) {
		 SPI.transfer( ((byte*)data)[msb-i] );
	}
}

/**
*	returns SUCCESS if the read failed
*	returns COMMERR if the computed check sum fails to make the ADE chksum
*/
uint8_t ADEgetRegister(ADEReg reg, int32_t *regValue)
{
	//get raw data, MSB of data is MSB from ADE irrespective of byte length
	uint8_t retCode = SUCCESS;
	uint32_t rawData = 0;
	uint8_t nBytes = (reg.nBits+7)/8;
	uint32_t chksum = 0;

	ADEreadData(reg, &rawData);
	ADEreadData(CHKSUM,&chksum);

	if (ADEchksum(rawData) != ((uint8_t*)&chksum)[3]) {
		retCode = COMMERR;
	}
	/*
	Serial1.print("ADEgetRegister rawData: ");
	Serial1.println(rawData,BIN);
	Serial1.print("ADEgetRegister chksum(rawdata): ");
	Serial1.println((int)ADEchksum(rawData));
	Serial1.print("ADEgetRegister chksum from ADE: ");
	Serial1.println(chksum,BIN);
	Serial1.print("ADEgetRegister chksum from ADE after shift: ");
	Serial1.println((int)(((uint8_t*)&chksum)[3]),BIN);
	Serial1.println(RCstr(retCode));
	*/
	//Push bits into MSB for irregular sizes
	rawData <<= (nBytes*8-reg.nBits);
	if (reg.signType == TWOS) {
		//Make signed
		*regValue = rawData;
		//Use signed shift for 8 byte alignment, then to move LSB to 0 byte
		(*regValue) >>= (nBytes*8-reg.nBits);
		(*regValue) >>= ((sizeof(regValue)-nBytes)*8);
	} else if (reg.signType == UNSIGN) {
		//Use unsigned shift for 8 byte alignment, then to move LSB to 0 byte
		rawData >>= (nBytes*8-reg.nBits);
		rawData >>= ((sizeof(rawData)-nBytes)*8);
		*regValue = rawData;
	} else {
		//Was for CH1OS/CH2OS not used
		/* if(reg.signType == SIGNMAG) {
		bool sign = rawData&(1<<(sizeof(uint32_t)-1));
		rawData &= ~(1<<(sizeof(uint32_t)-1));
		if (sign) { //MSB is one
			*regValue = -rawData;
		} else {
			*regValue = rawData;
		}*/
	}

	/*Serial1.println("ADEgetRegister EXIT");*/
	return retCode;
}


uint8_t ADEsetRegister(ADEReg reg, int32_t *value)
{
	uint8_t retCode = SUCCESS;
	uint32_t rawData;
	uint8_t nBytes = (reg.nBits+7)/8;

	if (reg.signType == TWOS || reg.signType == UNSIGN) {
		rawData = *value;
		//rawData <<= (nBytes*8-reg.nBits);
		rawData <<= ((sizeof(rawData)-nBytes)*8);
	} else { //Do nothing
		retCode = FAILURE;
	}
	//write data
	ADEwriteData(reg, &rawData);

	//read data and verify
	uint32_t readData;
	ADEreadData(reg,&readData);
	if (rawData != readData) {
		retCode = COMMERR;
	}
	
	return retCode;
}

/**
*	@return sum of 1 bits in data
*/
uint8_t ADEchksum(uint32_t data) 
{
	uint8_t sum = 0;
	for (uint8_t i=0; i < sizeof(data)*8; i++) {
		sum += data & 0x01;
		data >>= 1;
	}
	return sum;
}

/**
	The first bit of the CHX0S register is a flag and the last 5 bits are a signed magnitude value.
	i.e. F_SBBBBB
	CH2 does not use the enableInt bit.
	@warning CH2 is a negative offset so positive values decrease CH2

	@return SUCCESS, ARGEVALUEERR or ADEgetRegister errors
*/
uint8_t ADEgetCHXOS(uint8_t X,int8_t *enableInt,int8_t *val) 
{
	int retCode = SUCCESS;
	int32_t data  = 0;

	if (X == 1) {
		retCode = ADEgetRegister(CH1OS, &data);
	} else if (X == 2){
		retCode = ADEgetRegister(CH2OS, &data);
	} else {
		retCode = ARGVALUEERR;
	}

	uint8_t msB = ((uint8_t*)(&data))[0];
	*enableInt = msB & 0x80;

	//The 6th bit is not used
	*val       = msB & 0x1f; 

	//Sign
	if (msB & 0x20) {
		*val = -(*val);
	} 
	return retCode;
}

/** TODO fix rollover with timeout code, the code can exit early or wait 30 days
  * @return 0,1 value of interrupt. Returns a negative error code if a failure occurs. 
  */
int8_t ADEreadInterrupt(uint16_t regMask)
{
	int32_t status;
	int8_t retCode = ADEgetRegister(RSTSTATUS,&status);
	if (retCode == SUCCESS) {
		return status & regMask;
	} else {
		return -retCode;
	}
}

/** Will wait at least waitTimems milliseconds before exiting.
  @return SUCCESS if interrupt was fired, FAILURE otherwise.

  */
int8_t ADEwaitForInterrupt(uint16_t regMask, uint16_t waitTimems)
{
	int32_t status;
	int8_t retCode = ADEgetRegister(RSTSTATUS,&status);
	uint32_t time = millis();
	unsigned long endTime = time + waitTimems;
	if (time > endTime) {
		//wait for rollover
		do {
			retCode = ADEgetRegister(RSTSTATUS,&status);
			if (retCode == SUCCESS && (status & regMask)) {
				return SUCCESS;
			} 
		} while (millis() > endTime);
	}
	//now time=millis() should be less than endTime unless time 
	//overflowed to be much less than endTime to the point
	//where it is more than waitTimems far away
	do {
		retCode = ADEgetRegister(RSTSTATUS,&status);
		if (retCode == SUCCESS && (status & regMask)) {
			return SUCCESS;
		} 
	} while (time=millis() < endTime && endTime-time < waitTimems);
	return FAILURE;
}

int8_t ADEsetModeBit(uint16_t regMask, uint8_t bit)
{
	int32_t mode;
	int8_t retCode;
	retCode = ADEgetRegister(MODE, &mode);
	if (retCode != SUCCESS) {
		return retCode;
	}

	mode = mode & ~regMask;
	if (bit != 0) {
		mode = mode | regMask;
	}

	return ADEsetRegister(MODE, &mode);
}

