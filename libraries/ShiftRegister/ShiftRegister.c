/** @file ShiftRegister.cpp
@author Javier Rosa

In order to make any changes to the shift register visible apart for clearing and enabling, latch must be called.

*/
#include "ShiftRegister.h"

#define CLOCK(pin) digitalWrite(pin,HIGH);digitalWrite(pin,LOW);
/**
*	Register output pins are initialized to outputs and cleared.
*	The register is then set disabled.
*/
void initShiftRegister(){
	pinMode(SERIN,OUTPUT);
	pinMode(NOTENABLE,OUTPUT);
	pinMode(LATCH,OUTPUT);
	pinMode(NOTCLR,OUTPUT);
	pinMode(SHIFTCLK,OUTPUT);
 
        digitalWrite(SERIN,LOW);
	digitalWrite(LATCH, LOW);
        digitalWrite(NOTCLR,LOW);
	digitalWrite(SHIFTCLK, LOW);
	setEnabled(true); //May not be needed
	clearShiftRegister();
	latch();
	setEnabled(false);
}

/** 
*	@brief Enable/disable register
*/
void setEnabled( boolean enabled ){
	digitalWrite(NOTENABLE, enabled?LOW:HIGH);
}

/**
*	@brief Manifest register contents on output pins
*/
void latch() {
	CLOCK(LATCH);
}

/** 
*	@brief Pushes one bit onto the register.
*/
inline void shiftBit( boolean bit ){
	digitalWrite(SERIN, bit?HIGH:LOW);
	CLOCK(SHIFTCLK);
}

/** 
*	@brief Pushes a set of bits as bytes onto the register.
*	
*	Pushes a set of bits as bytes onto the register.
*	If bits[i] is not zero then the register line i is set to one.
*
*	@todo specify endianness
*/
void shiftArray( byte bits[] , uint8_t size){
	int idx;
	for(idx = 0; idx < size; idx++){
		shiftBit(bits[idx] != 0);
	}
}

/**
*	@brief Clear register
*/
void clearShiftRegister()
{
	//Clear when LOW
	digitalWrite(NOTCLR,LOW);
	digitalWrite(NOTCLR,HIGH);
}

/**
*	@brief Exercises ShiftRegister functions.
*
*	Initializes, pushes 101, latches, clears, 
*	sends an array of alternative 0s and 1s except for 
*	three 1s at the end.
*/
void testShiftRegister() {
	byte testArray[21] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,-7,12};

	init();

	//Enable and push 1,0,1, then test latch
	setEnabled(true);
	shiftbit(true);
	shiftbit(false);
	shiftbit(true);
	latch();

	//test clear
	delay(1000);
	clear();
	latch();

	//test shiftArray
	shiftbit(true);
	shiftbit(false);
	shiftbit(true);
	latch();
	delay(1000);
	
	shiftArray(testArray,21);
	latch();
	delay(1000);
	clear();
	latch();
	setEnabled(false);
}

