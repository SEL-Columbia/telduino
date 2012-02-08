#include <stdint.h>
#include "cfg.h"

int8_t buffCursor = 0;
char serBuff[serBuffSize];

int16_t reportInterval=10;     /** How often to report in seconds */
int8_t mode = INTERACTIVEMODE;  /** 0 emergency, 1 interactive, 2 meter */

// In memory and in EEPROM storage for circuit configuration
Circuit ckts[NCIRCUITS];
Circuit EEMEM cktsSave[NCIRCUITS];

