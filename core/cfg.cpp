#include <stdint.h>
#include "cfg.h"

int16_t reportInterval=10;     /** How often to report in seconds */
int8_t mode = INTERACTIVEMODE;  /** 0 emergency, 1 interactive, 2 meter */

// In memory and in EEPROM storage for circuit configuration
Circuit ckts[NCIRCUITS];
Circuit EEMEM cktsSave[NCIRCUITS];


int32_t EEMEM RARAASave[RARAASIZE][2] = {0}; //Used for a hacked up long running test
int32_t EEMEM nRARAASave = 0; //Total number of saved entries in RARAASave
