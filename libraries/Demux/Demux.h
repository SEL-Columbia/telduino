#ifndef DEMUX_H
#define DEMUX_H
#include "arduino/wiring.h"

#ifdef __cplusplus
extern "C" {
#endif

// demux pins
#define NENABLED 53 //This is also the main SS for SPI on the atmega 1280
#define A	30  //LSB
#define B	31
#define C	32
#define D	33
#define E	34
#define F	35
#define G	36  //Note only 21 circuits are actually active


void initDemux();
/* set enabled/disabled
 * The caller should make sure that the SD card is not enabled!
 */
void muxSetEnabled(boolean enabled);
/*
 * set line
 */
boolean muxSelect( int8_t line );

void testDemux();

#ifdef __cplusplus
}
#endif

#endif
