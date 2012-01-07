#ifndef TELDUINO_H
#define TELDUINO_H

#include <avr/eeprom.h>
#include "Circuit/circuit.h"
#include "Select/select.h"
#include "arduino/WProgram.h"

/** Definition of serial ports for debug, cpu communication, and telit communication
 *  @warning ensure consistence with the print, printD, printC, macros.
*/
#define dbg Serial
#define cpu Serial2
#define mdm Serial3

#define DEBUG_BAUD_RATE 9600
#define SHEEVA_BAUD_RATE 9600
#define TELIT_BAUD_RATE 9600

extern Circuit ckts[NCIRCUITS];
extern Circuit EEMEM cktsSave[NCIRCUITS];

void setup();
void loop();
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

#endif
