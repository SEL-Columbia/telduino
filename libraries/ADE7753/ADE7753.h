#ifndef ADE7753_H
#define ADE7753_H

#include "SPI/SPI.h"
//extern "C" {
	#include <inttypes.h> //check dir structure for the wiring lib -AM
//}

typedef struct {
	const char* name;
	const uint8_t addr;
	const uint8_t nbits;
	const bool signd;
} ADEReg;

//register defines -AM
const ADEReg WAVEFORM	= {"WAVEFORM",0x01,24,true};
const ADEReg AENERGY	= {"AENERGY",0x02,24,true};
const ADEReg RAENERGY	= {"RAENERGY",0x03,24,true};
const ADEReg LAENERGY	= {"LAENERGY",0x04,24,true};
const ADEReg VAENERGY	= {"VAENERGY",0x05,24,false};
const ADEReg RVAENERGY	= {"RVAENERGY",0x06,24,false};
const ADEReg LVAENERGY	= {"LVAENERGY",0x07,24,false};
const ADEReg LVARENERGY	= {"LVARENERGY",0x08,24,true};
const ADEReg MODE	= {"MODE",0x09,16,false};
const ADEReg IRQEN	= {"IRQEN",0x0A,16,false};
const ADEReg STATUS	= {"STATUS",0x0B,16,false};
const ADEReg RSTSTATUS	= {"RSTSTATUS",0x0C,16,false};
const ADEReg CH1OS	= {"CH1OS",0x0D,8,true }; //* signed not two's complement
const ADEReg CH2OS	= {"CH2OS",0x0E,8,true }; //* signed not two's complement
const ADEReg GAIN	= {"GAIN",0x0F,8,false};
const ADEReg PHCAL	= {"PHCAL",0x10,6,true};
const ADEReg APOS	= {"APOS",0x11,16,true};
const ADEReg WGAIN	= {"WGAIN",0x12,12,true};
const ADEReg WDIV	= {"WDIV",0x13,8,false};
const ADEReg CFNUM	= {"CFNUM",0x14,12,false};
const ADEReg CFDEN	= {"CFDEN",0x15,12,false};
const ADEReg IRMS	= {"IRMS",0x16,24,false};
const ADEReg VRMS	= {"VRMS",0x17,24,false};
const ADEReg IRMSOS	= {"IRMSOS",0x18,12,true};
const ADEReg VRMSOS	= {"VRMSOS",0x19,12,true};
const ADEReg VAGAIN	= {"VAGAIN",0x1A,12,true};
const ADEReg VADIV	= {"VADIV",0x1B,8,false};
const ADEReg LINECYC	= {"LINECYC",0x1C,16,false};
const ADEReg ZXTOUT	= {"ZXTOUT",0x1D,12,false};
const ADEReg SAGCYC	= {"SAGCYC",0x1E,8,false};
const ADEReg SAGLVL	= {"SAGLVL",0x1F,8,false};
const ADEReg IPKLVL	= {"IPKLVL",0x20,8,false};
const ADEReg VPKLVL	= {"VPKLVL",0x21,8,false};
const ADEReg IPEAK	= {"IPEAK",0x22,24,false};
const ADEReg RSTIPEAK	= {"RSTIPEAK",0x23,24,false};
const ADEReg VPEAK	= {"VPEAK",0x24,24,false};
const ADEReg TEMP	= {"TEMP",0x26,8,true};
const ADEReg PERIOD	= {"PERIOD",0x27,16,false};
const ADEReg TMODE	= {"TMODE",0x3D,8,false};
const ADEReg CHKSUM	= {"CHKSUM",0x3E,6,false};
const ADEReg DIEREV	= {"DIEREV",0x3F,8,false};

///*
//const ADEReg *aardvark[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };
//*/

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
bool readData(ADEReg,byte data[3]);
bool writeData(int8_t numBits, int8_t regAddr, uint8_t data[3]);

#endif
