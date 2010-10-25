#include "ShiftRegister.h"

/*
Author: Javier Rosa
Version: 0.0.1
Notes:

Need to see what the correct endianness is
*/

//TPIC6B959 shift register on pins:
#define NOTENABLE 28    //NOTG
#define LATCH     27    //RCK 
#define NOTCLR    26    //NOTSRCLR
#define SHIFTCLK  25    //SRCK
#define SERIN     29    //INPUT

//#define WIDTH     24    //WIDTH
//#define REVERSE   0     //Reverse order of shift

#define CLOCK(pin) digitalWrite(pin,HIGH);digitalWrite(pin,LOW);

void initShiftRegister(){
	pinMode(NOTENABLE,OUTPUT);
	pinMode(LATCH,OUTPUT);
	pinMode(NOTCLR,OUTPUT);
	pinMode(SHIFTCLK,OUTPUT);
	pinMode(SERIN,OUTPUT);
 
	digitalWrite(SHIFTCLK, LOW);
	digitalWrite(LATCH, LOW);
	setEnabled(true);
	clear();
	latch();
	setEnabled(false);
}

//enable/disable register
void setEnabled( boolean enabled ){
	digitalWrite(NOTENABLE, enabled?LOW:HIGH);
}

//latch register contents on output pins
void latch() {
	CLOCK(LATCH);
}

//only one bit do not latch
inline void shiftBit( boolean bit ){
	digitalWrite(SERIN, bit?LOW:HIGH);
	CLOCK(SHIFTCLK);
}

//does not latch
void shiftArray( byte bits[] , uint8_t size){
	int idx;
	for(idx = 0; idx < size; idx++){
		shiftBit(bits[idx] != 0);
	}
}

//clear register
void clearShiftRegister(){
	CLOCK(NOTCLR);
}

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
