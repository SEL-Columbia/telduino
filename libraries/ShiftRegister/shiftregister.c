/** @file shiftregister.c
@author Javier Rosa

In order to make any changes to the shift register visible apart for clearing and enabling, latch must be called.

*/
#include "shiftregister.h"

#define CLOCK(pin) digitalWrite(pin,HIGH);digitalWrite(pin,LOW);
/**
*	Register output pins are initialized to outputs and cleared.
*	The register is then set disabled.
*/
void SRinit(){
	pinMode(SERIN,OUTPUT);
	pinMode(NOTENABLE,OUTPUT);
	pinMode(LATCH,OUTPUT);
	pinMode(NOTCLR,OUTPUT);
	pinMode(SHIFTCLK,OUTPUT);
 
	digitalWrite(SERIN,LOW);
	digitalWrite(LATCH, LOW);
	digitalWrite(NOTCLR,LOW);
	digitalWrite(SHIFTCLK, LOW);
	SRsetEnabled(true); 
	SRclear();
	SRlatch();
	SRsetEnabled(false);
}

/** 
*	@brief Enable/disable register
*/
void SRsetEnabled(boolean enabled){
	digitalWrite(NOTENABLE, enabled?LOW:HIGH);
}

/**
*	@brief Manifest register contents on output pins
*/
void SRlatch() {
	CLOCK(LATCH);
}

/** 
*	@brief Pushes one bit onto the register.
*/
void SRshiftBit( boolean bit ){
	digitalWrite(SERIN, bit?HIGH:LOW);
	CLOCK(SHIFTCLK);
}

/** 
*	@brief Pushes a set of bits specified by booleans onto the register.
*	Pushes a set of bits specified by booleans onto the register. If bits[i] is 0 then the register line i is set to 1. Therefore, bits[0] is the last bit shifted into the register.
*
*/
void SRshiftArray(byte bits[] , int8_t size){
	for(size = size-1; size >= 0; size--){
		SRshiftBit(bits[size] != 0);
	}
}

/**
*	@brief Clear register
*/
void SRclear()
{
	//Clear when LOW
	digitalWrite(NOTCLR,LOW);
	digitalWrite(NOTCLR,HIGH);
}

void SRall1() 
{
	SRclear();
	uint8_t bits[] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
	SRshiftArray(bits, WIDTH);
}

/**
*	@brief Exercises ShiftRegister functions.
*
*	Initializes, pushes 101, latches, clears, 
*	sends an array of alternative 0s and 1s except for 
*	three 1s at the end.
*/
void SRtest() {
	byte testArray[21] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,-7,12};

	SRinit();

	//Enable and push 1,0,1, then test latch
	SRsetEnabled(true);
	SRshiftbit(true);
	SRshiftbit(false);
	SRshiftbit(true);
	SRlatch();

	//test clear
	delay(1000);
	SRclear();
	SRlatch();

	//test shiftArray
	SRshiftbit(true);
	SRshiftbit(false);
	SRshiftbit(true);
	SRlatch();
	delay(1000);
	
	SRshiftArray(testArray,21);
	SRlatch();
	delay(1000);
	SRclear();
	SRlatch();
	SRsetEnabled(false);
}

