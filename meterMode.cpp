#include "meterMode.h"
#include "cfg.h"
#include "Circuit/circuit.h"

const char *FMTSTRINGI = "%c %d %d\r";

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

//GET
//(s)witch status
//(m)ode status 
//(r)eport Interval in seconds

//! do nothing NOP

void parseMeterMode(char *cmd) {
    char action = '\r';
    int8_t cktID = NCIRCUITS + 1;
    int32_t arg = 0;

    //TODO Sanitize input
    if ( sscanf(cmd,FMTSTRINGI,&action, &cktID, &arg) != 3){
        printResults('!',21,-1);
        return;
    }
    
    switch (action) {
        case 'S':
            if (cktID == -1) {
                for (int8_t id = MAINS+1; id<NCIRCUITS;id++) {
                    CsetOn(&ckts[id],arg);
                }
            } else {
                CsetOn(&ckts[cktID],arg);
            }
            break;
        case 'M':
            mode = arg;
            break;
        case 'R':
            reportInterval = arg;
            break;
        case 's':
            arg = CisOn(&ckts[cktID]);
            break;
        case 'm':
            arg = mode;
            break;
        case 'r':
            arg = reportInterval;
            break;
    }
    printResults(action,cktID,arg);

}

void meterAll() {
    //Is it time?
    //Resett all ADE LINCYCs
    //Print ckt value
    //Print out sequence values
    //Print out time stamp
    
}

void printResults(char action, int8_t cktID, int32_t arg) {
    cpu.print(action);
    cpu.print(" ");
    cpu.print(cktID);
    cpu.print(" ");
    cpu.println(arg);
}


