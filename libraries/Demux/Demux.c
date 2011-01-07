#include "Demux.h"

/**
 * Mux starts off disabled and set to have output 0 LOW and all others HIGH.
 */
void initDemux(){
	pinMode(A,OUTPUT);
	pinMode(B,OUTPUT);
	pinMode(C,OUTPUT);
	pinMode(D,OUTPUT);
	pinMode(E,OUTPUT);
	pinMode(F,OUTPUT);
	pinMode(G,OUTPUT);
	pinMode(NENABLED,OUTPUT);
	
	muxSetEnabled(false);
        muxSelect(0);
}

void muxSetEnabled(boolean enabled){
        digitalWrite(NENABLED, enabled?LOW:HIGH);
}
	
//Lines are numbered from 0 to 20
//The last circuit line 20 is physcially the mains circuit
int8_t muxSelect( int8_t line ){
	//The hex pair X,Y corresponding to number 0-20 (00 - A5) defines the physcial circuit being activated.
	//for line 20, the last/main line, DCBA=1111 HGFE=0101
        muxSetEnabled(false);
	if ( 0 <= line && line < 15){
		//This just converts line into binary input to the mux.
		digitalWrite(A, (line & 0x01)?HIGH:LOW);
		digitalWrite(B, (line & 0x02)?HIGH:LOW);
		digitalWrite(C, (line & 0x04)?HIGH:LOW);
		digitalWrite(D, (line & 0x08)?HIGH:LOW);
		//EFG are irrelevant as the second muxer is disabled until ABCD=1111. The demuxers have all of their output set to high if the input is high.
		//As the muxers are chained via the last output if the first muxer is not set to 1111 the second has all of its lines high.
                
		//Just to keep the timings the same
		digitalWrite(E, LOW);
		digitalWrite(F, LOW);
		digitalWrite(G, LOW);
	} else if (15 <= line && line < 21) {
		//See comment above. The first muxer is set to enable the second muxer.
		digitalWrite(A, HIGH);
		digitalWrite(B, HIGH);
		digitalWrite(C, HIGH);
		digitalWrite(D, HIGH);
		
		line -= 15;
		digitalWrite(E, (line & 0x01)?HIGH:LOW);
		digitalWrite(F, (line & 0x02)?HIGH:LOW);
		digitalWrite(G, (line & 0x04)?HIGH:LOW);
	} else {
		//TODO: This is more an error than anything else.
        muxSetEnabled(false);
		return ARGVALUEERR;
	}
        muxSetEnabled(true);
	return SUCCESS;
}

void testDemux() {
	int ii = 0;
	initDemux();

	muxSelect(20); //Mains
	
	//Enable/Disable CS
	for (ii=0; ii < 10; ii++) {
		muxSetEnabled(true);
		delay(500);
		muxSetEnabled(false);
	}

	muxSetEnabled(true);

	for (ii=0; ii < 21; ii++) {
		muxSelect(ii);
		delay(2000);
	}
}
