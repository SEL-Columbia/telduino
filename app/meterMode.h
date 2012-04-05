#ifndef METERMODE_H
#define METERMODE_H
#include <stdint.h>
#include "Circuit/circuit.h"
 
extern long nextMeter;
void parseMeterMode(char *cmd);
void printMeter(Circuit* ckt);
void meter(Circuit* ckt);
void meterAll();
void meterAuto();
void printResults(char action, int8_t circuitID, int32_t arg);

#endif
