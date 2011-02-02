#include "arduino/WProgram.h"
#include "arduino/wiring.h"
#include "SPI/SPI.h"
#include "ReturnCode/returncode.h"

#include "ADE7753.h"

const ADEReg WAVEFORM  = {"WAVEFORM",0x01,24,TWOS};
const ADEReg AENERGY	= {"AENERGY",0x02,24,TWOS};
const ADEReg RAENERGY    = {"RAENERGY",0x03,24,TWOS};
const ADEReg LAENERGY    = {"LAENERGY",0x04,24,TWOS};
const ADEReg VAENERGY    = {"VAENERGY",0x05,24,UNSIGN};
const ADEReg RVAENERGY   = {"RVAENERGY",0x06,24,UNSIGN};
const ADEReg LVAENERGY   = {"LVAENERGY",0x07,24,UNSIGN};
const ADEReg LVARENERGY  = {"LVARENERGY",0x08,24,TWOS};
const ADEReg MODE    = {"MODE",0x09,16,UNSIGN};
const ADEReg IRQEN   = {"IRQEN",0x0A,16,UNSIGN};
const ADEReg STATUS  = {"STATUS",0x0B,16,UNSIGN};
const ADEReg RSTSTATUS   = {"RSTSTATUS",0x0C,16,UNSIGN};
const ADEReg CH1OS   = {"CH1OS",0x0D,8,UNSIGN}; //* signed mag two's complement
const ADEReg CH2OS   = {"CH2OS",0x0E,8,UNSIGN}; //* signed mag not two's complement
const ADEReg GAIN    = {"GAIN",0x0F,8,UNSIGN};
const ADEReg PHCAL   = {"PHCAL",0x10,6,TWOS};
const ADEReg APOS    = {"APOS",0x11,16,TWOS};
const ADEReg WGAIN   = {"WGAIN",0x12,12,TWOS};
const ADEReg WDIV    = {"WDIV",0x13,8,UNSIGN};
const ADEReg CFNUM   = {"CFNUM",0x14,12,UNSIGN};
const ADEReg CFDEN   = {"CFDEN",0x15,12,UNSIGN};
const ADEReg IRMS    = {"IRMS",0x16,24,UNSIGN};
const ADEReg VRMS    = {"VRMS",0x17,24,UNSIGN};
const ADEReg IRMSOS  = {"IRMSOS",0x18,12,TWOS};
const ADEReg VRMSOS  = {"VRMSOS",0x19,12,TWOS};
const ADEReg VAGAIN  = {"VAGAIN",0x1A,12,TWOS};
const ADEReg VADIV   = {"VADIV",0x1B,8,UNSIGN};
const ADEReg LINECYC = {"LINECYC",0x1C,16,UNSIGN};
const ADEReg ZXTOUT  = {"ZXTOUT",0x1D,12,UNSIGN};
const ADEReg SAGCYC  = {"SAGCYC",0x1E,8,UNSIGN};
const ADEReg SAGLVL  = {"SAGLVL",0x1F,8,UNSIGN};
const ADEReg IPKLVL  = {"IPKLVL",0x20,8,UNSIGN};
const ADEReg VPKLVL  = {"VPKLVL",0x21,8,UNSIGN};
const ADEReg IPEAK   = {"IPEAK",0x22,24,UNSIGN};
const ADEReg RSTIPEAK    = {"RSTIPEAK",0x23,24,UNSIGN};
const ADEReg VPEAK   = {"VPEAK",0x24,24,UNSIGN};
const ADEReg TEMP    = {"TEMP",0x26,8,TWOS};
const ADEReg PERIOD  = {"PERIOD",0x27,16,UNSIGN};
const ADEReg TMODE   = {"TMODE",0x3D,8,UNSIGN};
const ADEReg CHKSUM  = {"CHKSUM",0x3E,6,UNSIGN};
const ADEReg DIEREV  = {"DIEREV",0x3F,8,UNSIGN};

const ADEReg *regList[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };
const int regListSize = sizeof(regList);

/**
* returns BYTES from the ADE in a uint32_t value
* MSB of ADE output is the MSB of the data output
* @warning SPI mode is changed after calling this funciton
*/
void ADEreadData(ADEReg reg, uint32_t *data)
{
    uint8_t nBytes = (reg.nBits+7)/8;

	SPI.setDataMode(SPI_MODE1);
    //SPI speed is f_osc/128
    SPI.setClockDivider(SPI_CLOCK_DIV128);

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
  resets _retCode to SUCCESS then tries to get the regValue from register reg
*/
void ADEgetRegister(ADEReg reg, int32_t *regValue)
{
	//Serial1.print("GET ");
	//Serial1.println(reg.name);
	//get raw data, MSB of data is MSB from ADE irrespective of byte length
	_retCode = SUCCESS;
	uint32_t rawData = 0;
	uint8_t nBytes = (reg.nBits+7)/8;
	uint32_t chksum = 0;
	int8_t retries = RETRIES;

	//Serial1.print("(int32_t)(&),HEX:");
	//Serial1.println((int32_t)(&WAVEFORM),HEX);

	ADEreadData(reg, &rawData);
	ADEreadData(CHKSUM,&chksum);
	if (ADEchksum(rawData) != ((uint8_t*)&chksum)[3]) {
		_retCode = COMMERR;
		//Serial1.print("ADE _retCode:");
		//Serial1.println(RCstr(_retCode));
	} else {
		_retCode = SUCCESS;
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
	Serial1.println(RCstr(_retCode));
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
		_retCode = FAILURE;
	}

	/*Serial1.println("ADEgetRegister EXIT");*/
}

/**
*	
*	resets _retCode to SUCCESS then tries to get the regValue from register reg
*	@warning Range of input value is not checked. Refer to the ADE7753 datasheet for proper input ranges.
  */
void ADEsetRegister(ADEReg reg, int32_t *value)
{
	//Serial1.print("SET ");
	//Serial1.println(reg.name);
	_retCode = SUCCESS;
	uint32_t writeData;
	uint8_t nBytes = (reg.nBits+7)/8;
	uint8_t shiftBits = nBytes*8-reg.nBits;
	//Serial1.println(shiftBits,DEC);

	if (reg.signType == TWOS || reg.signType == UNSIGN) {
		writeData = *value;
		//rawData <<= (nBytes*8-reg.nBits);
		writeData <<= ((sizeof(writeData)-nBytes)*8);
	} else { 
		//Do nothing this shouldn't happen
		//Serial1.println("MAJOR PROBLEM");
		_retCode = FAILURE;
	}
	//write data
	//Serial1.println("writtenData: ");
	//Serial1.println(writeData,HEX);
	ADEwriteData(reg, &writeData);
	//Serial1.println(writeData<<shiftBits,HEX);

	//read data and verify
	uint32_t readData;
	ADEreadData(reg,&readData);
	//Serial1.println("readData   : ");
	//Serial1.println(readData,HEX);
	//Serial1.println(readData<<shiftBits,HEX);
	if ((writeData<<shiftBits) != (readData<<shiftBits)) {
		_retCode = COMMERR;
	}
	
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

*/
void ADEgetCHXOS(const uint8_t X,int8_t *enableInt,int8_t *val) 
{
	int32_t data  = 0;

	if (X == 1) {
		ADEgetRegister(CH1OS, &data);
	} else if (X == 2){
		ADEgetRegister(CH2OS, &data);
	} else {
		_retCode = ARGVALUEERR;
	}

	uint8_t msB = ((uint8_t*)(&data))[0];
	*enableInt = msB & 0x80;

	//The 6th bit is not used
	*val       = msB & 0x1f; 

	//Sign
	if (msB & 0x20) {
		*val = -(*val);
	} 
}

void ADEsetCHXOS(const uint8_t X,const int8_t *enableInt,const int8_t *val) 
{
	int32_t data  =*val;

	//convert to signed magnitude
	if (data < 0) {
		data = -data;
		data |= 0x20;
	}

	data &= ~0x80;
	if (enableInt && X == 1) {
		data |= 0x80;
	}

	if (X == 1) {
		ADEsetRegister(CH1OS, &data);
	} else if (X == 2){
		ADEsetRegister(CH2OS, &data);
	} else {
		_retCode = ARGVALUEERR;
	}
}

/** 
  * @return 1 if interrupt is fired. Returns 0 otherwise or if a failure occurs. 
  */
int8_t ADEreadInterrupt(uint16_t regMask)
{
	int32_t status;
	ADEgetRegister(RSTSTATUS,&status);
	ifsuccess(_retCode) {
		if (regMask == ZX0) {
			status = ~status;
			regMask = ZX;
		}
		return ((status & regMask) != 0);
	} else {
		return 0;
	}
}

/** Will wait at least waitTimems milliseconds before exiting.
  _retCode is SUCCESS if interrupt was fired, TIMEOUT otherwise.
  @warning does not reset register, that is the users responsibility

  */
void ADEwaitForInterrupt(uint16_t regMask, uint16_t waitTimems)
{
	int32_t status = 0;
	unsigned long time = millis();
	unsigned long endTime = time + waitTimems;
	_retCode = SUCCESS;
	if (time > endTime) {
		//wait for rollover
		do {
			if (ADEreadInterrupt(regMask)) return;
			ifnsuccess(_retCode) {Serial1.println("COMMERR in waitForInterrupt");}
		} while (millis() > endTime);
	}
	//now time=millis() should be less than endTime unless time 
	//overflowed to be much less than endTime to the point
	//where it is more than waitTimems far away
	do {
		if (ADEreadInterrupt(regMask)) return;
		ifnsuccess(_retCode) { Serial1.println("COMMERR in waitForInterrupt");}
		time = millis();
	} while ((time <= endTime) && (endTime-time <= waitTimems));
	/*
	Serial1.print("waitforInterrupt waiting(ms):");
	Serial1.println(waitTimems);
	Serial1.print("waitforInterrupt time(ms):");
	Serial1.println(time);
	Serial1.print("waitforInterrupt endTime(ms):");
	Serial1.println(endTime);
	*/
	_retCode = TIMEOUT;
}

/**
	Reads the mode register changes the bit specified by bitMask to be 
	0 of bit is zero and 1 if  bit is otherwise.
	@return SUCCESS if bit has been changed.
  */
void ADEsetModeBit(uint16_t regMask, uint8_t bit)
{
	int32_t mode;
	ADEgetRegister(MODE, &mode);
	ifnsuccess(_retCode) return;

	mode = mode & ~regMask;
	if (bit != 0) {
		mode = mode | regMask;
	}

	ADEsetRegister(MODE, &mode);
}

void ADEsetIrqEnBit(uint16_t regMask, uint8_t bit)
{
	int32_t irq;
	ADEgetRegister(IRQEN, &irq);
	ifnsuccess(_retCode) return;

	irq = irq & ~regMask;
	if (bit != 0) {
		irq = irq | regMask;
	}

	ADEsetRegister(IRQEN, &irq);
}

/**
    The ADE returns a period value with a resolution of 2.2us/LSbit.
	  This function returns that value.
	    */
int8_t ADEperToFreq(int32_t period)
{
    return (uint8_t)(10000000/(period*22)); 
}

/**
	Perform a software reset of the ADE. It must be reprogrammed afterwards.
	I don't believe this actually does anything.
	@warning This assumes that at most a 16mhz clock is being used.
  */
void ADEreset()
{
	//Writes the defaults+the software reset bit
	uint32_t regData = 0x004c0000;
	ADEwriteData(MODE,&regData);
	for (int i=0; i<500; i++);//Wait at least 32us assuming a 16mhz clock
}


