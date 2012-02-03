/** @file DbgTel.cpp */
#include "DbgTel.h"

/**Must be called before setDbgLeds.*/
void initDbgTel()
{
	pinMode(GRNPIN,OUTPUT);
	pinMode(YELPIN,OUTPUT);
	pinMode(REDPIN,OUTPUT);
}

/**
    Bit 0,1,2 in pattern is G,Y,R.
*/
void setDbgLeds(int8_t pattern) 
{
	/*Just light up the LEDS for now.*/
	if (pattern & RPAT) {
		digitalWrite(REDPIN, HIGH);
	} else {
		digitalWrite(REDPIN, LOW);
	
	}

	if (pattern & GPAT) {
		digitalWrite(GRNPIN, HIGH);
	} else {
		digitalWrite(GRNPIN, LOW);
	}

	if (pattern & YPAT) {
		digitalWrite(YELPIN, HIGH);
	} else {
		digitalWrite(YELPIN, LOW);
	}
}

