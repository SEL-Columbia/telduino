#ifndef ADECALIBRATION_H
#define ADECALIBRATION_H
#ifdef __cplusplus
extern "C"
{
#endif

void calibrateCircuit(uint8_t circuitID);
int8_t CLgetString(HardwareSerial *ser,char *buff, size_t bSize);
int8_t CLgetFloat(HardwareSerial *ser,float *f);
int8_t CLgetInt(HardwareSerial *ser,int*i);


#ifdef __cplusplus
}
#endif
#endif
