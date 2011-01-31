#ifndef ADE7753_H
#define ADE7753_H

#include <stdint.h> 
#ifdef TWOS
#error "TWOS is defined"
#endif
/**
* The type of number retrieved from the ADE
*/
enum {UNSIGN=0,SIGNMAG=1,TWOS=2};
//NOTE: We can recover memory (3*41= 124 bytes) by just using a single, data buffer



typedef struct {
	const char* name;
	const uint8_t addr;
	const uint8_t nBits;
	const uint8_t signType;
	uint8_t data[3]; 
} ADEReg;

//register defines -AM
//These are effectively constant as the struct has constant members
static const ADEReg WAVEFORM	= {"WAVEFORM",0x01,24,TWOS};
static const ADEReg AENERGY	= {"AENERGY",0x02,24,TWOS};
static const ADEReg RAENERGY	= {"RAENERGY",0x03,24,TWOS};
static const ADEReg LAENERGY	= {"LAENERGY",0x04,24,TWOS};
static const ADEReg VAENERGY	= {"VAENERGY",0x05,24,UNSIGN};
static const ADEReg RVAENERGY	= {"RVAENERGY",0x06,24,UNSIGN};
static const ADEReg LVAENERGY	= {"LVAENERGY",0x07,24,UNSIGN};
static const ADEReg LVARENERGY	= {"LVARENERGY",0x08,24,TWOS};
static const ADEReg MODE	= {"MODE",0x09,16,UNSIGN};
static const ADEReg IRQEN	= {"IRQEN",0x0A,16,UNSIGN};
static const ADEReg STATUS	= {"STATUS",0x0B,16,UNSIGN};
static const ADEReg RSTSTATUS	= {"RSTSTATUS",0x0C,16,UNSIGN};
static const ADEReg CH1OS	= {"CH1OS",0x0D,8,UNSIGN}; //* signed mag two's complement
static const ADEReg CH2OS	= {"CH2OS",0x0E,8,UNSIGN}; //* signed mag not two's complement
static const ADEReg GAIN	= {"GAIN",0x0F,8,UNSIGN};
static const ADEReg PHCAL	= {"PHCAL",0x10,6,TWOS};
static const ADEReg APOS	= {"APOS",0x11,16,TWOS};
static const ADEReg WGAIN	= {"WGAIN",0x12,12,TWOS};
static const ADEReg WDIV	= {"WDIV",0x13,8,UNSIGN};
static const ADEReg CFNUM	= {"CFNUM",0x14,12,UNSIGN};
static const ADEReg CFDEN	= {"CFDEN",0x15,12,UNSIGN};
static const ADEReg IRMS	= {"IRMS",0x16,24,UNSIGN};
static const ADEReg VRMS	= {"VRMS",0x17,24,UNSIGN};
static const ADEReg IRMSOS	= {"IRMSOS",0x18,12,TWOS};
static const ADEReg VRMSOS	= {"VRMSOS",0x19,12,TWOS};
static const ADEReg VAGAIN	= {"VAGAIN",0x1A,12,TWOS};
static const ADEReg VADIV	= {"VADIV",0x1B,8,UNSIGN};
static const ADEReg LINECYC	= {"LINECYC",0x1C,16,UNSIGN};
static const ADEReg ZXTOUT	= {"ZXTOUT",0x1D,12,UNSIGN};
static const ADEReg SAGCYC	= {"SAGCYC",0x1E,8,UNSIGN};
static const ADEReg SAGLVL	= {"SAGLVL",0x1F,8,UNSIGN};
static const ADEReg IPKLVL	= {"IPKLVL",0x20,8,UNSIGN};
static const ADEReg VPKLVL	= {"VPKLVL",0x21,8,UNSIGN};
static const ADEReg IPEAK	= {"IPEAK",0x22,24,UNSIGN};
static const ADEReg RSTIPEAK	= {"RSTIPEAK",0x23,24,UNSIGN};
static const ADEReg VPEAK	= {"VPEAK",0x24,24,UNSIGN};
static const ADEReg TEMP	= {"TEMP",0x26,8,TWOS};
static const ADEReg PERIOD	= {"PERIOD",0x27,16,UNSIGN};
static const ADEReg TMODE	= {"TMODE",0x3D,8,UNSIGN};
static const ADEReg CHKSUM	= {"CHKSUM",0x3E,6,UNSIGN};
static const ADEReg DIEREV	= {"DIEREV",0x3F,8,UNSIGN};

/**
	Interrupt Register MASKS
*/
static const uint16_t AEHF		= 0x0001; //bit 0
static const uint16_t SAG		= 0x0002; //bit 1
static const uint16_t CYCEND	= 0x0004; //bit 2
static const uint16_t WSMP		= 0x0008; //bit 3
static const uint16_t ZX		= 0x0010; //bit 4
static const uint16_t ZX0		= 0x0011; //Wait for a 0 not a 1
static const uint16_t TEMPREG	= 0x0020; //bit 5 ---I changed this from TEMP to TEMPREG because of the ADEReg with the same name.
static const uint16_t RESET	= 0x0040; //bit 6
static const uint16_t AEOF		= 0x0080; //bit 7
static const uint16_t PKV		= 0x0100; //bit 8
static const uint16_t PKI		= 0x0200; //bit 9 
static const uint16_t VAEHF	= 0x0400; //bit 10 A
static const uint16_t VAEOF	= 0x0800; //bit 11 B
static const uint16_t ZXTO		= 0x1000; //bit 12 C
static const uint16_t PPOS		= 0x2000; //bit 13 D
static const uint16_t PNEG		= 0x4000; //bit 14 E


/**
	Mode (MODE) Register MASKS
*/
static const uint16_t DISHPF	= 0x0001; //bit 0
static const uint16_t DISHPF2 	= 0x0002; //bit 1
static const uint16_t DISCF  	= 0x0004; //bit 2
static const uint16_t DISSAG  	= 0x0008; //bit 3
static const uint16_t ASUSPEND	= 0x0010; //bit 4
static const uint16_t TEMPSEL	= 0x0020; //bit 5
static const uint16_t SWRST	= 0x0040; //bit 6
static const uint16_t CYCMODE	= 0x0080; //bit 7
static const uint16_t DISCH1	= 0x0100; //bit 8
static const uint16_t DISCH2	= 0x0200; //bit 9 
static const uint16_t SWAP	= 0x0400; //bit 10 A
static const uint16_t DTRT_0	= 0x0800; //bit 11 B
static const uint16_t DTRT1_	= 0x1000; //bit 12 C
static const uint16_t WAVESEL_0= 0x2000; //bit 13 D
static const uint16_t WAVESEL1_= 0x4000; //bit 14 E

/**
	Binary two-tuple constants
*/
static const uint8_t ZZ=0b00;
static const uint8_t ZO=0b01;
static const uint8_t OZ=0b10;
static const uint8_t OO=0b11;

static const ADEReg *regList[] = { &WAVEFORM, &AENERGY, &RAENERGY, &LAENERGY, &VAENERGY, &RVAENERGY, &LVAENERGY, &LVARENERGY, &MODE, &IRQEN, &STATUS, &RSTSTATUS, &CH1OS, &CH2OS, &GAIN, &PHCAL, &APOS, &WGAIN, &WDIV, &CFNUM, &CFDEN, &IRMS, &VRMS, &IRMSOS, &VRMSOS, &VAGAIN, &VADIV, &LINECYC, &ZXTOUT, &SAGCYC, &SAGLVL, &IPKLVL, &VPKLVL, &IPEAK, &RSTIPEAK, &VPEAK, &TEMP, &PERIOD, &TMODE, &CHKSUM, &DIEREV };

/**
 * @warning SPI.begin() must have already been called before any of these commands are used
 * @warning the CS line for the associated ADE must be low and others high
 */
void ADEreadData(ADEReg, uint32_t*);
void ADEwriteData(ADEReg, uint32_t*);
void ADEgetRegister(ADEReg, int32_t*);
void ADEsetRegister(ADEReg, int32_t*);
uint8_t ADEchksum(uint32_t);
void ADEgetCHXOS(const uint8_t X,int8_t *enableBit,int8_t *val);
void ADEsetCHXOS(const uint8_t X,const int8_t *enableInt,const int8_t *val);
int8_t ADEreadInterrupt(uint16_t regMask);
void ADEwaitForInterrupt(uint16_t regMask, uint16_t waitTimems);
void ADEsetModeBit(uint16_t regMask, uint8_t bit);
int8_t ADEperToFreq(int32_t period);
void ADEreset();

#define RETRIES 3
//Sample circuit functions
//Get stuff

#endif

