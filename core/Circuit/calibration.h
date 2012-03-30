#ifndef ADECALIBRATION_H
#define ADECALIBRATION_H
#include "circuit.h"
#ifdef __cplusplus
extern "C"
{
#endif

void calibrateCircuit(Circuit *c);
int8_t CLgetString(HardwareSerial *ser,char *buff, size_t bSize);
int8_t CLgetFloat(HardwareSerial *ser,float *f);
int8_t CLgetInt(HardwareSerial *ser,int32_t*i);
int32_t avg(int n, int32_t (*sample)(void*), void* context, int32_t *var);

#ifdef __cplusplus
}
#endif
#endif
