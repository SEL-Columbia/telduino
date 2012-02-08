#ifndef TELDUINO_H
#define TELDUINO_H

#include <avr/eeprom.h>
#include "Circuit/circuit.h"
#include "Select/select.h"
#include "arduino/WProgram.h"
#include "cfg.h"

void setup();
void loop();
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

#endif
