#include "switches.h"
#include "ReturnCode/returncode.h"

/**
    Mapping from switch number to pins.
*/
static const int8_t mapSWtoPinON[] = {11,56};
static const int8_t mapSWtoPinOFF[] = {55,54};

/**
  The workhorse of the Switches library. This function along with 
  _SWsetSwitches(), SWinit, and _enabledC is used to implement the actual switching.
*/
inline void _SWset(int8_t sw, int8_t on) 
{
    _enabledC[sw] = on;
    int8_t pinOn = mapSWtoPinON[sw];
    int8_t pinOff = mapSWtoPinOFF[sw];
    if (on) {
        digitalWrite(pinOn,LOW);
    } else {
        digitalWrite(pinOff,LOW);
    }
    //TODO use a real delay function to wait 10ms
    //This assumes a 16mhz 8-bit cpu
    for (int32_t i=0; i<20000; i++);
    digitalWrite(pinOn,HIGH);
    digitalWrite(pinOff,HIGH);
}

/**
  Normally, this function can just call _SWset. 
  If the switch is controlled by shift registers, that might not be
  optimal.

*/
inline void _SWsetSwitches() 
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
        digitalWrite(pinOn,HIGH);
        digitalWrite(pinOff,HIGH);
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
	Turns all circuits off. This implies that the actual relays are turning on.
  */
void SWallOff()
{
    int8_t i = 0;
    for (i = 0; i < NSWITCHES; i++) {
         _enabledC[i] = false;
    }
    _SWsetSwitches();
}

/** 
	Turns all circuits on. This implies that the actual relays are turning off.
  */
void SWallOn()
{
    int8_t i;
    for (i = 0; i < NSWITCHES; i++) {
        _enabledC[i] = true;
    }
    _SWsetSwitches();
}

/**
	Returns an array of size NSWITCHES.	If entry 0 is 1 then switch 0 is on
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

