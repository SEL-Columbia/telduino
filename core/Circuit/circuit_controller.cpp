#include "circuit.h"
#include "ADE7753/ADE7753.h"

char* PARAMETERS[3] = {"Wslope","VAslope","CsetSampleTime"};
CSET* CSETS[3] = {CsetWslope,CsetVAslope,CsetSampleTime};

void CsetWslope(Circuit *c, float Wslope) 
{
    c->Wslope = Wslope;    
}

void CsetVAslope(Circuit *c, float VAslope) 
{
    c->VAslope = VAslope;    
}

/**
 *  Updates the LINECYC register and c->cyclesSample to be the 
 *  correct number of cycles to contain "ms" milliSeconds of samples.
 *  This uses the period information gained from the last successful reading.
 *  
 *  Assuming 40-70Hz ms can have a value of 
 */
void CsetSampleTime(Circuit *c,float ms)
{
    int32_t linecycval;
    ms*=1000;
    ms*=1000;
    c->cyclesSample = (uint16_t)ms/c->periodus;
    linecycval = c->cyclesSample*2;
    ADEsetRegister(LINECYC,&linecycval);
}

