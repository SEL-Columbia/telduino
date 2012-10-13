#ifndef METERMODE_H
#define METERMODE_H
#include <stdint.h>
#include "Circuit/circuit.h"
 
#define SERBUFFSIZE 64
extern float sampleTime_ms;

void meterMode();
void parseMeterMode(char *cmd);
void printMeter(Circuit* ckt);
void meter(Circuit* ckt);
void meterAll();
void meterAuto();
void printResults(char action, int8_t circuitID, int32_t arg);

#endif
