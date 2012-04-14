#include "switches.h"
#include "ReturnCode/returncode.h"

/**
 * @file Switches.cpp
 *
 * TODO: Since the relays are latching relays, switch to High-Z.
 * */


/**
    Mapping from switch number to pins.
*/
static const int8_t mapSWtoPinON[] = {11,56,10,66,74,22,73,27,3,81,48,15,76,34,75,41,77,20,13,45};
static const int8_t mapSWtoPinOFF[] = {55,54,65,64,83,69,26,25,82,39,14,78,33,32,40,37,70,71,84,85};

/**
  The workhorse of the Switches library. This function along with 
  _SWsetSwitches(), SWinit, and _enabledC is used to implement the actual switching.
*/
void _SWset(int8_t sw, int8_t on) 
{
    _enabledC[sw] = on;
    int8_t pinOn = mapSWtoPinON[sw];
    int8_t pinOff = mapSWtoPinOFF[sw];
    if (on) {
        digitalWrite(pinOn,HIGH);
    } else {
        digitalWrite(pinOff,HIGH);
    }
    // Relays need 10ms
	delay(10);
    digitalWrite(pinOn,LOW);
    digitalWrite(pinOff,LOW);
}

/**
  Normally, this function can just call _SWset. 
  If the switch is controlled by shift registers, that might not be
  optimal.
*/
void _SWsetSwitches() 
{
    for (int8_t i=0; i < NSWITCHES; i++) {
        _SWset(i,_enabledC[i]);
    }
}

/** Configures appropriate pins and sets pins to match _enabledC[].
*/
void SWinit() 
{
    for(int8_t i=0; i<NSWITCHES; i++) {
        int8_t pinOn = mapSWtoPinON[i];
        int8_t pinOff = mapSWtoPinOFF[i];
        pinMode(pinOn,OUTPUT);
        pinMode(pinOff,OUTPUT);
        digitalWrite(pinOn,LOW);
        digitalWrite(pinOff,LOW);
        _SWset(i,_enabledC[i]);
    }
}


/**
    There are NSWITCHES switches in the circuit.
    if enabledC[i] == 1 then the circuit is on and off if it is 0.
*/
void SWsetSwitches(int8_t enabledC[NSWITCHES]) 
{
    int8_t i = 0;
    for (i =0; i < NSWITCHES; i++) {
        _enabledC[i] = enabledC[i];
    }
    _SWsetSwitches();
}

/**
  For any non-zero value of on the switch is turned on. 
  0 <= sw < NSWITCHES
  */
void SWset(int8_t sw, int8_t on) 
{
    if (0 <= sw && sw < NSWITCHES) {
        _SWset(sw, on);
    } else {
        _retCode = ARGVALUEERR;
    }
}

/** 
	Turns all circuits off.
  */
void SWallOff()
{
    int8_t i = 0;
    for (i = 0; i < NSWITCHES; i++) {
		_SWset(i,false);
    }
}

/** 
	Turns all circuits on.
  */
void SWallOn()
{
    int8_t i;
    for (i = 0; i < NSWITCHES; i++) {
		_SWset(i,true);
    }
}

/**
	@return an array of size NSWITCHES.	If entry 0 is 1 then switch 0 is on.
  */
const int8_t* SWgetSwitchState()
{
   return _enabledC;
}

/**
	@return true if sw is on, false otherwise
  */
uint8_t SWisOn(int8_t sw)
{
    if (0<=sw && sw<NSWITCHES) {
        return _enabledC[sw] != 0;
    } else {
        _retCode = ARGVALUEERR;
        return false;
    }
}

