#ifndef DEMUX_H
#define DEMUX_H
#include <stdint.h>

#define NCIRCUITS  21

// demux pins
#define NENABLED 53 //This is also the main SS for SPI on the atmega 1280
#define A	30		//LSB
#define B	31
#define C	32
#define D	33
#define E	34
#define F	35
#define G	36		//Note only 21 circuits are actually active so this pin is not needed


#ifdef __cplusplus
extern "C"{
#endif

void initDemux();
/* set enabled/disabled
 * The caller should make sure that the SD card is not enabled!
 */
void muxSetEnabled(uint8_t enabled);
/*
 * set line
 */
void muxSelect(int8_t line);

void testDemux();

#ifdef __cplusplus
}
#endif
#endif
