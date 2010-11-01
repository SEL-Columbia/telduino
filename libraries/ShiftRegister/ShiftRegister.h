/** @file ShiftRegister.h 

@author: Javier Rosa
@todo Need to see what a sane endianness is.
@todo decide if we should assume sanitized inputs

*/

#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

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
void setEnabled( boolean enabled );
void latch();
inline void shiftBit( boolean bit );
void shiftArray( byte bits[] , uint8_t size);
void clearShiftRegister();
void testShiftRegister();


#endif
