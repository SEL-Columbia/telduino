#ifndef DBGTEL_H
#define DBGTEL_H

#include <inttypes.h>
#include "arduino/wiring.h"
/** @file DbgTel.h
*	There are three debug leds
*
*	Green  G
*	Yellow Y
*	Red    R
*	
*	A pattern is specified with a three digit number GYR where G,Y,R are 0,1 to specifify if that LED is on or off. In the future the PWM functionality of those pins will be used for more complex signals.
*
*/

enum LedPattern { OFFPAT = 0, GPAT = 1,  YPAT = 10, RPAT = 100, GYRPAT = 111, GYPAT = 11, GRPAT = 101, YRPAT = 110 };

/**
	Arduino pin numbers for the LEDs.
	These are PWM pins.
*/
const int8_t GRNPIN = 44;
const int8_t YELPIN = 45;
const int8_t REDPIN = 46;

void initDbgTel();
void setDbgLeds(int8_t pattern);

#endif
