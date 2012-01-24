#include <stdint.h>
#include "cfg.h"

extern char serBuff[128];

int16_t reportInterval=120;     /** How often to report in seconds */
int8_t mode = INTERACTIVEMODE;  /** 0 emergency, 1 interactive, 2 meter */

// In memory and in EEPROM storage for circuit configuration
Circuit ckts[NCIRCUITS];
Circuit EEMEM cktsSave[NCIRCUITS];

