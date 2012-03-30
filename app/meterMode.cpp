#include <stdint.h>
#include "meterMode.h"
#include "cfg.h"
#include "Circuit/circuit.h"
#include "arduino/wiring.h"
#include "ReturnCode/returncode.h"


/** Command packet format (every packet has these fields)
 * COMMANDCHAR CIRCUIT NUMBER\r
 * COMMANDCHAR is a single character which determines the action.
 * CIRCUIT is an integer from -1 through 19, 
 *  -1 applies the command to all circuits when relevant.
 * NUMBER is an integer for now
 *
 * The response is in the same format if only to echo what was given.
 * A command error returns a !, an impossible circuit id, and a -1 
*/

//Commands
//SET
//(S)witch set
//(M)ode
//(R)eport Interval in seconds
//(W)atts meter ALL circuits

//GET
//(s)witch status
//(m)ode status 
//(r)eport Interval in seconds
//(w)atts meter circuit

//! do nothing NOP
uint64_t lastMeter = 0;
int32_t sequenceNum = 0;
const char *FMTSTRINGI = "%c %hd %ld";

void parseMeterMode(char *cmd) 
{
    char action = '!';
    int8_t cktID = NCIRCUITS + 1;
    int32_t arg = 0;

    //TODO Sanitize input

    if ( sscanf(cmd,FMTSTRINGI,&action, &cktID, &arg) != 3) {
        printResults('!',21,-1);
        return;
    }
    
    switch (action) {
        case 'S':
            if (cktID == -1) {
                for (int8_t id = 0; id<NCIRCUITS;id++) {
                    if (id == MAINS) continue;
                    CsetOn(&ckts[id],arg);
                }
            } else {
                CsetOn(&ckts[cktID],arg);
            }
            break;
        case 'M':
            mode = arg;
            break;
        case 'T':
            reportInterval = arg;
            break;
        case 'W':
            meterAll();
            break;
        case 'w':
            meter(&ckts[cktID]);
            break;
        case 's':
            arg = CisOn(&ckts[cktID]);
            break;
        case 'm':
            arg = mode;
            break;
        case 't':
            arg = reportInterval;
            break;
        default:
            action = '!';
            break;
    }
    printResults(action,cktID,arg);

}

void meterAuto() 
{   

    //TODO assumes millis returns an unsigned long
    //TODO save data regularly in nonvolitile memory
    uint64_t timeNow = millis();
    // TODO overflow case
    //if (timeNow < lastMeter) {
    //    timeNow = (uint32_t(-1))-(lastMeter-timeNow); //Now time diference
    //} else {
    timeNow = timeNow - lastMeter; //Now time diference
    //}
    if (reportInterval > 0 && (timeNow/1000 < reportInterval)) {
        return;
    }
    lastMeter = millis();
    meterAll();
}

void meter(Circuit *ckt)
{
    Cmeasure(ckt);
    printMeter(ckt);
}
void meterAll() 
{
    //Prepare all ckts for reading. If the code starts to rely on LINCYC, this can become counterproductive.
    dbg.println("ts,seq,#ID,S,V,I,Vp,Ip,per,VA,W,VAE,WE,PF,0,0,Errcode");
    for (int i=0; i < NCIRCUITS; i++) {
        Cclear(&ckts[i]);
    }
    for (int i=0; i < NCIRCUITS; i++) {
        meter(&ckts[i]);
    }
}

void printMeter(Circuit *ckt) {
    RCreset();
    cpu.print(millis());
    cpu.print(",");
    cpu.print(sequenceNum++);
    cpu.print(",");
    CprintMeas(&cpu,ckt);
    cpu.print(",");
    cpu.print(_retCode);
    cpu.println();
}

void printResults(char action, int8_t cktID, int32_t arg) {
    cpu.print(action);
    cpu.print(" ");
    cpu.print(cktID);
    cpu.print(" ");
    cpu.println(arg);
}

