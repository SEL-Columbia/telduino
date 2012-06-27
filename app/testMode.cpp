#include "testMode.h"
#include <stdint.h>
#include "Switches/switches.h"
#include "Select/select.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"
#include "ReturnCode/returncode.h"
#include "cfg.h"

//Relay burn out until stop metering voltage to detect missed switchings
//Relay burn in to detect missed switchings
//Metering continuously
//Fast switching for EM problems
void testMode() 
{

}


/**
  Tests the switch for 5 seconds on each of 4 different switch speeds.
 */
void testSwitch(int8_t swID)
{
    int times[]      = {2500,1000, 500, 200,  10};
    int switchings[] = {   2,   5,  10,  50, 500};
    for (int i=0; i < sizeof(times)/sizeof(times[0]); i++) {
        for (int j=0; j < switchings[i]; j++){
            SWset(swID, true);
            delay(times[i]/2);
            SWset(swID, false);
            delay(times[i]/2);
        }
    }
}

void burnInRelays() 
{
    int nTests = 1000;
    int IRMSthresh = 10000;
    int failures[NSWITCHES][2];
    int32_t val;
    for (int i=0; i < NSWITCHES; i++) {
        failures[i][0] = failures[i][1] =0;
    }
    for (int i=0; i<1000; i++) {
        CSselectDevice(DEVDISABLE);
        //Start turning each switch off measure current then on and messure current
        SWallOff();
        for (int i=0; i<NSWITCHES; i++) {
            CSselectDevice(i);
            ADEgetRegister(IRMS,&val);
            ifnsuccess(_retCode) {
                dbg.print("Comm failure on ");
                dbg.print(i); dbg.print(":");
                dbg.println(RCstr(_retCode));
            } else if (val> IRMSthresh) {
                failures[i][0]++;
                dbg.print("Failure turing off relay "); dbg.println(i);
            }
            RCreset();
            CSselectDevice(DEVDISABLE);
        }
        SWallOn();
        for (int i=0; i<NSWITCHES; i++) {
            CSselectDevice(i);
            ADEgetRegister(IRMS,&val);
            ifnsuccess(_retCode) {
                dbg.print("Comm failure on ");
                dbg.print(i); dbg.print(":");
                dbg.println(RCstr(_retCode));
            } else if (val> IRMSthresh) {
                failures[i][1]++;
                dbg.print("Failure turing on relay "); dbg.println(i);
            }
            RCreset();
            CSselectDevice(DEVDISABLE);
        }
    }
    dbg.println("relay,off_failures,on_failures");
    for (int i=0; i < NSWITCHES; i++) {
        dbg.print(i);dbg.print(","); dbg.print(failures[i][0]);dbg.print(",");dbg.println(failures[i][1]);
    }
}
/** 
  Quickly turns on all circuits.
  Then turns off all of them as fast as possible except for MAINS.
  Then tries to communicate with the ADEs.
 */
void testSwitching() 
{
    int8_t enabledC[NSWITCHES] = {0};
    int32_t val;

    dbg.print("\n\rTest switches\n\r");

    SWallOn();
    delay(1000);
    SWallOff();

    //Start turning each switch on with 1 second in between
    for (int i = 0; i < NSWITCHES; i++) {
        enabledC[i] = 1;
        delay(1000);
        SWset(enabledC[i], true);
        //SWsetSwitches(enabledC);
    }
    delay(1000);
    SWallOff();

    //Test communications with each ADE
    for (int i = 0; i < NCIRCUITS; i++) {
        CSselectDevice(i);

        dbg.print("Can communicate with channel ");
        dbg.print(i,DEC);
        dbg.print(": ");

        ADEgetRegister(DIEREV,&val);
        ifnsuccess(_retCode) {
            dbg.print("NO-");
            dbg.println(RCstr(_retCode));
        } else {
            dbg.print("YES-DIEREV:");
            dbg.println(val,DEC);
        }
        CSselectDevice(DEVDISABLE);
    }
}
