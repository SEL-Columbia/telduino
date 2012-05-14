#include <stdint.h>
#include "meterMode.h"
#include "cfg.h"
#include "Circuit/circuit.h"
#include "arduino/wiring.h"
#include "ReturnCode/returncode.h"


/** 
 *  \mainpage Meter Mode
 *
 *  \section Purpose
 *      Manages the restricted set of metering functionality provided
 *      by the meter when operating autmoatically.
 *
 *  \section Sending a Command
 *       Command packet format (every packet has these fields)
 *       COMMANDCHAR CIRCUIT NUMBER\r
 *       COMMANDCHAR is a single character which determines the action.
 *       CIRCUIT is an integer from -1 through 19, 
 *        -1 applies the command to all circuits when relevant.
 *       NUMBER is an integer for now
 *      
 *       The response is in the same format if only to echo what was given.
 *       A command error returns: "! -1 -1"
 *
 *  \section Commands
 *      SET
 *      (S)witch set
 *      (M)ode
 *      (R)eport Interval in seconds
 *      (W)atts meter ALL circuits
 *
 *      GET
 *      (s)witch status
 *      (m)ode status 
 *      (r)eport Interval in seconds
 *      (w)atts meter circuit
 *      ! do nothing NOP
 *
 *  \section TODO
 *      parseMeterMode: TODO (X)Reset and reprogram Meter x
 *      parseMeterMode: TODO Sanitize input
 *      meterAuto: TODO save data regularly in nonvolitile memory
 *      meterAuto: TODO if measure fails attempt to restore communications, 
 *      meterAll: TODO In the future if we assume that the delay 
 *          between outputs is equal to LINECYC 
 *      we can just wait on LINECYC every time instead of clearing first.
 *      meterAuto:  TODO if first meter reset after the second one 
 *          is read, then reset and reprogram
 * */

/** Serial Buffer for meterMode. */
char serBuff[SERBUFFSIZE];
/** Keeps track of the serial buffer head. */
int8_t buffCursor = 0;

/**Last time a metering occured. Used to determine when next time will occur.*/
uint64_t lastMeterTime_ms = 0;
/** This will roll over after 2^32-1*/
uint32_t sequenceNum = 0;
/**Command format string.*/
const char *FMTSTRINGI = "%c %hd %ld";

/**
* Entry point for meter mode. Handles input from CPU Serial line while in metermode.
*/
void meterMode() 
{
    if (cpu.available()) {
        char c = cpu.read();
        if (buffCursor < (SERBUFFSIZE-1)) {
            serBuff[buffCursor] = c;
           if (c == '\r' || c == '\n') {
                serBuff[buffCursor] = '\0';
                buffCursor = 0;
                if (cpu.peek() == '\n' || cpu.peek() == '\r') {
                    cpu.read();
                }   
                parseMeterMode(serBuff);
            } else {
                buffCursor += 1;
            }
        } else {
            buffCursor = 0;
            serBuff[0] = '\0';
        }
    }
    /*If there are no actions to take. See if metering is needed.*/
    meterAuto();
}


/**
 * Handles the meterMode command after it has been successfully scanned.
 * */
void parseMeterMode(char *cmd) 
{
    char action = '!';
    int8_t cktID = NCIRCUITS + 1;
    int32_t arg = 0;


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

/**
 * If enough time has passed printout all meter data.
 * */
void meterAuto() 
{   

    uint32_t timeNow = millis();
    uint32_t diff = timeNow - lastMeterTime_ms;
    if (timeNow < lastMeterTime_ms) { //overflow
        diff = ((uint32_t(-1))-lastMeterTime_ms)+timeNow; 
    } 
    if (reportInterval > 0 && (diff/1000 < reportInterval)) {
        return;
    } 
    lastMeterTime_ms = millis();
    if (reportInterval < 0) {
        return;
    }
    meterAll();
}

/**Meters and prints the results of the metering operation.*/
void meter(Circuit *ckt)
{
    RCreset();
    Cmeasure(ckt);
    printMeter(ckt);
}

/**
 *  Pulls data form all of the meters by first clearing their LINECYC interrupts.
 */
void meterAll() 
{
    //Prepare all ckts for reading. If the code starts to rely on LINCYC, this can become counterproductive.
    dbg.println("ts,seq,#ID,S,V,I,Vp,Ip,per,VA,W,VAE,WE,PF,0,0,StatusCode");
    for (int i=0; i < NCIRCUITS; i++) {
        Cclear(&ckts[i]);
    }
    for (int i=0; i < NCIRCUITS; i++) {
        meter(&ckts[i]);
    }
}

/** 
 *  Outputs all of the metering data when requested by a command or polled.
 */
void printMeter(Circuit *ckt) {
    cpu.print(millis());
    cpu.print(",");
    cpu.print(sequenceNum++);
    cpu.print(",");
    CprintMeas(&cpu,ckt);
    cpu.print(",");
    cpu.print(ckt->status,HEX);
    cpu.println();
}

/**
 * Prints the result of a command. Which is the same syntax 
 * as the given command.
 * */
void printResults(char action, int8_t cktID, int32_t arg) {
    cpu.print(action);
    cpu.print(" ");
    cpu.print(cktID);
    cpu.print(" ");
    cpu.println(arg);
}

