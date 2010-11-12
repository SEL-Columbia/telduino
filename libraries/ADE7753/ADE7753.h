#ifndef ADE7753_H
#define ADE7753_H

#include <inttypes.h> 
#include "SPI/SPI.h"	

/**
* The type of number retrieved from the ADE
*/
enum {UNSIGN=0,SIGNMAG=1,TWOS=2};
//NOTE: We can recover memory (3*41= 124 bytes) by just using a single, data buffer

typedef struct {
	const char* name;
	const uint8_t addr;
	const uint8_t nBits;
	const bool signType;
	byte data[3]; 
} ADEReg;

//register defines -AM
//These are effectively constant as the struct has constant members
const ADEReg WAVEFORM	= {"WAVEFORM",0x01,24,TWOS};
const ADEReg AENERGY	= {"AENERGY",0x02,24,TWOS};
const ADEReg RAENERGY	= {"RAENERGY",0x03,24,TWOS};
const ADEReg LAENERGY	= {"LAENERGY",0x04,24,TWOS};
const ADEReg VAENERGY	= {"VAENERGY",0x05,24,UNSIGN};
const ADEReg RVAENERGY	= {"RVAENERGY",0x06,24,UNSIGN};
const ADEReg LVAENERGY	= {"LVAENERGY",0x07,24,UNSIGN};
const ADEReg LVARENERGY	= {"LVARENERGY",0x08,24,TWOS};
const ADEReg MODE	= {"MODE",0x09,16,UNSIGN};
const ADEReg IRQEN	= {"IRQEN",0x0A,16,UNSIGN};
const ADEReg STATUS	= {"STATUS",0x0B,16,UNSIGN};
const ADEReg RSTSTATUS	= {"RSTSTATUS",0x0C,16,UNSIGN};
const ADEReg CH1OS	= {"CH1OS",0x0D,8,SIGNMAG }; //* signed mag two's complement
const ADEReg CH2OS	= {"CH2OS",0x0E,8,SIGNMAG}; //* signed mag not two's complement
const ADEReg GAIN	= {"GAIN",0x0F,8,UNSIGN};
const ADEReg PHCAL	= {"PHCAL",0x10,6,TWOS};
const ADEReg APOS	= {"APOS",0x11,16,TWOS};
const ADEReg WGAIN	= {"WGAIN",0x12,12,TWOS};
const ADEReg WDIV	= {"WDIV",0x13,8,UNSIGN};
const ADEReg CFNUM	= {"CFNUM",0x14,12,UNSIGN};
const ADEReg CFDEN	= {"CFDEN",0x15,12,UNSIGN};
const ADEReg IRMS	= {"IRMS",0x16,24,UNSIGN};
const ADEReg VRMS	= {"VRMS",0x17,24,UNSIGN};
const ADEReg IRMSOS	= {"IRMSOS",0x18,12,TWOS};
const ADEReg VRMSOS	= {"VRMSOS",0x19,12,TWOS};
const ADEReg VAGAIN	= {"VAGAIN",0x1A,12,TWOS};
const ADEReg VADIV	= {"VADIV",0x1B,8,UNSIGN};
const ADEReg LINECYC	= {"LINECYC",0x1C,16,UNSIGN};
const ADEReg ZXTOUT	= {"ZXTOUT",0x1D,12,UNSIGN};
const ADEReg SAGCYC	= {"SAGCYC",0x1E,8,UNSIGN};
const ADEReg SAGLVL	= {"SAGLVL",0x1F,8,UNSIGN};
const ADEReg IPKLVL	= {"IPKLVL",0x20,8,UNSIGN};
const ADEReg VPKLVL	= {"VPKLVL",0x21,8,UNSIGN};
const ADEReg IPEAK	= {"IPEAK",0x22,24,UNSIGN};
const ADEReg RSTIPEAK	= {"RSTIPEAK",0x23,24,UNSIGN};
const ADEReg VPEAK	= {"VPEAK",0x24,24,UNSIGN};
const ADEReg TEMP	= {"TEMP",0x26,8,TWOS};
const ADEReg PERIOD	= {"PERIOD",0x27,16,UNSIGN};
const ADEReg TMODE	= {"TMODE",0x3D,8,UNSIGN};
const ADEReg CHKSUM	= {"CHKSUM",0x3E,6,UNSIGN};
const ADEReg DIEREV	= {"DIEREV",0x3F,8,UNSIGN};
///*
//const ADEReg *aardvark[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };
//*/

/**
 * @warning SPI.begin() must have already been called
 * @warning the CS line for the associated ADE must be low and others high
 */
bool initADE(); 

/**
*  
*/
int readData(ADEReg, uint32_t*);
int ADEgetRegister(ADEReg, int32_t*);
int chksum(uint32_t);
//uint32_t uxtu32_t(uint8_t data[3],int nBytes);
#endif

