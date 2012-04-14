#ifndef CFG_H
#define CFG_H

#include <avr/eeprom.h>
#include "Circuit/circuit.h"

/** select.h contains lower level hardware configuration information 
 * like how many circuits there are*/
#include "Select/select.h"


/** Definition of serial ports for debug, cpu communication, and telit communication
    @warning ensure consistence with the print, printD, printC, macros.
*/
#define dbg Serial
#define cpu Serial
//#define cpu Serial2
#define mdm Serial3
#define DBG_BAUD_RATE 9600
#define CPU_BAUD_RATE 9600
#define MDM_BAUD_RATE 9600

//HACKED UP TEST REMOVE
#define RARAASIZE 225

#define MAINS 0
extern int16_t reportInterval;  /** How often to report in seconds */
extern int8_t mode;             /** 0 emergency, 1 interactive, 2 meter, 3 test */
#define EMERGENCYMODE 0
#define INTERACTIVEMODE 1
#define METERMODE 2
#define TESTMODE 3

extern Circuit ckts[NCIRCUITS];
//EEPROM DATA
extern Circuit EEMEM cktsSave[NCIRCUITS];
//HACKED UP TEST REMOVE \/
extern int32_t EEMEM RARAASave[RARAASIZE][2];
extern int32_t EEMEM nRARAASave;
            
#endif
