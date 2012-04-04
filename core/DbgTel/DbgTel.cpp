/** @file DbgTel.cpp */
#include "DbgTel.h"
#include "arduino/wiring.h"
#include "cfg.h"

void DbgTelInit()
{
	pinMode(GRNPIN,OUTPUT);
	pinMode(YELPIN,OUTPUT);
	pinMode(REDPIN,OUTPUT);
}

void DbgLeds(int8_t pattern) 
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

void DbgPrint(char *tag, char *label, int32_t value )
{
    dbg.print(tag);
    dbg.print(":");
    dbg.print(label);
    dbg.print(":");
    dbg.println(value,BIN);
    dbg.print(":");
    dbg.println(value,HEX);
    dbg.print(":");
    dbg.print(value);
}
