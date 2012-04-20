#include "circuit.h"
char* PARAMETERS[2] = {"Wslope","VAslope"};
CSET* CSETS[2] = {CsetWslope,CsetVAslope};

void CsetWslope(Circuit *c, float Wslope) 
{
    c->Wslope = Wslope;    
}

void CsetVAslope(Circuit *c, float VAslope) 
{
    c->VAslope = VAslope;    
}
