#ifndef TELDUINO_H
#define TELDUINO_H

#include "arduino/wiring.h"

void setup();
void loop();
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

#endif
