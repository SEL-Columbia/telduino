#ifndef TELDUINO_H
#define TELDUINO_H

#include <avr/eeprom.h>
#include "Circuit/circuit.h"
#include "Select/select.h"
#include "arduino/WProgram.h"

//definition of serial ports for debug, sheeva communication, and telit communicatio
#define debugPort Serial
#define sheevaPort Serial2
#define telitPort Serial3

#define DEBUG_BAUD_RATE 9600
#define SHEEVA_BAUD_RATE 9600
#define TELIT_BAUD_RATE 9600

extern Circuit ckts[NCIRCUITS];
extern Circuit EEMEM cktsSave[NCIRCUITS];

void setup();
void loop();
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

#endif
