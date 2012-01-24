#ifndef METERMODE_H
#define METERMODE_H
#include <stdint.h>

void parseMeterMode(char *cmd);
void printResults(char action, int8_t circuitID, int32_t arg);

#endif
