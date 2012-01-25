#ifndef METERMODE_H
#define METERMODE_H
#include <stdint.h>
#include "Circuit/circuit.h"

extern long nextMeter;

void parseMeterMode(char *cmd);
void meter(Circuit* ckt);
void meterAll();
void printResults(char action, int8_t circuitID, int32_t arg);

#endif
