/** @file ShiftRegister.h 

@author: Javier Rosa
@todo Need to see what a sane endianness is.

*/

#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include <inttypes.h>
#include "arduino/wiring.h"


//TPIC6B959 shift register on pins:
#define NOTENABLE 28    //NOTG
#define LATCH     27    //RCK 
#define NOTCLR    26    //NOTSRCLR
#define SHIFTCLK  25    //SRCK
#define SERIN     29    //INPUT

#define WIDTH     24    //WIDTH
#define REVERSE   0     //Reverse order of shift

#define CLOCK(pin) digitalWrite(pin,HIGH);digitalWrite(pin,LOW);

void initShiftRegister();

//enable/disable register
void setEnabled( boolean enabled );

//latch register contents on output pins
void latch();

//only one bit do not latch
inline void shiftBit( boolean bit );


//does not latch
void shiftArray( byte bits[] , uint8_t size);

//clear register
void clearShiftRegister();

void testShiftRegister();
#endif
