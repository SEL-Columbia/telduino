/** @file DbgTel.cpp */
#include "DbgTel.h"

void initDbgTel()
{
	pinMode(GRNPIN,OUTPUT);
	pinMode(YELPIN,OUTPUT);
	pinMode(REDPIN,OUTPUT);
}

void setDbgLeds(int8_t pattern) 
{
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

