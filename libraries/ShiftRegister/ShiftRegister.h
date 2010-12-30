// vim: autoindent:smartindent:tabstop=4:shiftwidth=4:noexpandtab

/** @file ShiftRegister.h 

@author: Javier Rosa
@todo Need to see what a sane endianness is.
@todo decide if we should assume sanitized inputs

*/
#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <inttypes.h>
#include "arduino/wiring.h"


/** 
* 
* TPIC6B959 shift register on pins:
* 	
*/
#define NOTENABLE 28    //NOTG
#define LATCH     27    //RCK 
#define NOTCLR    26    //NOTSRCLR
#define SHIFTCLK  25    //SRCK
#define SERIN     29    //INPUT

//#define WIDTH     24    //WIDTH
//#define REVERSE   0     //Reverse order of shift

void initShiftRegister();
void setEnabled( int8_t enabled );
void latch();
void shiftBit( int8_t bit );
void shiftArray( byte bits[] , uint8_t size);
void clearShiftRegister();
void testShiftRegister();


#ifdef __cplusplus
    }
#endif
#endif
