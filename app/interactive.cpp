/**
 *  \mainpage Interactive Mode
 *
 *  \section Purpose
 *    Manages interactive CLI interface for Telduino
 *
 *  \section Implementation
 *      Later 
 *  
 */

#include <stdlib.h>
#include <stdint.h>

#include <avr/wdt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

//Helper functions
#include "ReturnCode/returncode.h"
#include "arduino/WProgram.h"
#include "Strings/strings.h"

//Metering Hardware
#include "DbgTel/DbgTel.h"
#include "ADE7753/ADE7753.h"
#include "Select/select.h"
#include "Switches/switches.h"
#include "sd-reader/sd_raw.h"

//Metering logic
#include "Circuit/circuit.h"
#include "Circuit/calibration.h"

#include "interactive.h"
#include "telduino.h"
#include "cfg.h"

//For interactive mode
int _testChannel = 1; //This is the input daughter board channel. This should only b

//Hacked up test
int32_t switchSec = 0;
int32_t testIdx = 0;//Present index into RARAASAVE counts down to 0
int32_t switchings = 0;


void testCircuitPrint() 
{
    int32_t records = 0;
    int32_t RARAA[2] = {0};
    //Get number of records from first block
    eeprom_read_block(&records,&nRARAASave,sizeof(records));

    //Iterate over these records and print in CSV format
    dbg.println();
    dbg.print("ON: RAENERGY,OFF:RAENERGY");
    dbg.println();
    for (int i = records-1; i >0 ; i--) {
        eeprom_read_block(RARAA,&RARAASave[i],sizeof(RARAA));
        dbg.print(RARAA[0]);
        dbg.print(",");
        dbg.print(RARAA[1]);
        dbg.println();
    }
}

int8_t blinkTime()
{
    //Check for comm errors
    ifnsuccess(_retCode) {
        //FAILURE
        for (int i=0; i < 10; i++) {
            setDbgLeds(GRPAT);
            //delay(100);
            setDbgLeds(OFFPAT);
            //delay(100);
        }
        RCreset();
        return true;
    }
    return false;
}

int8_t blinkComm()
{
    //Check for comm errors
    ifnsuccess(_retCode) {
        //FAILURE
        for (int i=0; i < 10; i++) {
            setDbgLeds(GRPAT);
            //delay(100);
            setDbgLeds(GPAT);
            //delay(100);
        }
        return true;
        RCreset();
    }
    return false;
}

/**
 *  Single character serial interface for interaction with telduino
 *  Capital letters are usually writes and lower case letters are usually reads
 */
void parseBerkeley() 
{
    dbg.println();
    dbg.print(_testChannel,DEC);
    dbg.print(" $");
    while (dbg.available() == 0 && testIdx == 0) {
        setDbgLeds(GPAT);
        for (int i=0; i < 100; i++) {
            if (dbg.available() != 0) {
                break;
            }
            delay(10);
        }
        setDbgLeds(0);
        for (int i=0; i < 100; i++) {
            if (dbg.available() != 0) {
                break;
            }
            delay(10);
        }
    }

    if (testIdx > 0) {
        uint32_t startTime = millis();
        uint32_t val = 0;
        int32_t RARAA[2] = {0};

        //Clear AEnergy
        ADEgetRegister(RAENERGY,&RARAA[1]);
        RCreset();

        //Switch the channel On
        ADEwaitForInterrupt(ZX0,20);
        SWset(_testChannel,true);
        switchings += 1;

        //Meter it 
        ADEwaitForInterrupt(CYCEND,1100);
        RCreset();
        ADEwaitForInterrupt(CYCEND,1100);
        if (blinkTime()){
            RARAA[0] = -2000;
        } else {
            ADEgetRegister(LAENERGY,&RARAA[0]);
        }
        if (blinkComm()) {
            RARAA[0] = -1000;
        }

        //Switch the channel Off
        ADEwaitForInterrupt(ZX0,10);
        SWset(_testChannel,false);
        RCreset();
        switchings += 1;
        //Meter it 
        ADEwaitForInterrupt(CYCEND,1050);
        ADEgetRegister(RAENERGY,&RARAA[1]);
        if (blinkComm()) {
            RARAA[1] = -1000;
        }
        
        dbg.print(RARAA[0]);
        dbg.print(",");
        dbg.print(RARAA[1]);

        //Write to EEPROM
        eeprom_update_block(RARAA,&RARAASave[testIdx],sizeof(RARAA));

        //Switch during delay between tests so interval is switchSec seconds
        uint32_t time = millis()-startTime;
        uint32_t remaining = 1000*switchSec-time;
        if (time > 1000*switchSec) {
            remaining = 0;
        } else {
            dbg.println();
        }

        const int rate = 6000;
        uint32_t switchStart = millis();
        while (remaining > rate) {
            SWset(_testChannel,true);
            delay(rate/2-10);
            SWset(_testChannel,false);
            delay(rate/2-10);
            switchings += 1;
            uint32_t switchTime = millis()-switchStart;
            if (remaining > switchTime) {
                remaining -= switchTime;
            } else {
                remaining -= rate;
            }
            switchStart = millis();
        }
        delay(remaining); 

        if (testIdx == 1){
            dbg.println();
            dbg.print("Testing Complete.");
            dbg.print("Switchings:");
            dbg.print(switchings);
            dbg.println();
        }
        testIdx -= 1;
    }
    // Look for incoming single character command on dbg line
    // Capital letters denote write operations and lower case letters are reads
    if (dbg.available() > 0) {
        char incoming = dbg.read(); 
        // TODO Make functions for each case instead of dumping code
        char buff[16] = {0};
        int32_t regData = 0;
        const char * codes[NCIRCUITS] = {};
        Circuit *c;
        int8_t ID;
        int32_t secs = 0;
        int32_t retVal = 0;
        int32_t mask = 0;
        int32_t linecycVal = 120;
        int32_t gain = 0x24;
        int32_t phcal = 0x0B;
        int32_t zeros[2] = {0};
        int32_t runMin = 0;
        dbg.println(incoming);
        switch (incoming) {
            case 'A':                       //Write to ADE Register
                dbg.print("Register to write $");
                CLgetString(&dbg,buff,sizeof(buff));
                dbg.println();

                for (int i=0; i < regListSize/sizeof(regList[0]); i++) {
                    if (strcmp(regList[i]->name,buff) == 0){
                        CSselectDevice(_testChannel);
                        dbg.print("Current regData:");
                        ADEgetRegister(*regList[i],&regData);
                        dbg.print(RCstr(_retCode));
                        dbg.print(":0x");
                        dbg.print(regData,HEX);
                        dbg.print(":");
                        dbg.println(regData,BIN);

                        dbg.print("Enter new regData:");
                        if(CLgetInt(&dbg,&regData) == CANCELED) break;
                        dbg.println();
                        ADEsetRegister(*regList[i],&regData);
                        dbg.print(RCstr(_retCode));
                        dbg.print(":0x");
                        dbg.print(regData,HEX);
                        dbg.print(":");
                        dbg.println(regData,DEC);
                        CSselectDevice(DEVDISABLE);
                        break;
                    }
                }
                break;
            case 'a':                       //Read ADE reg
                dbg.print("Enter name of register to read:");
                CLgetString(&dbg,buff,sizeof(buff));
                dbg.println();

                //dbg.print("(int32_t)(&),HEX:");
                //dbg.println((int32_t)(&WAVEFORM),HEX);
                for (int i=0; i < regListSize/sizeof(regList[0]); i++) {
                    if (strcmp(regList[i]->name,buff) == 0){
                        CSselectDevice(_testChannel);
                        ADEgetRegister(*regList[i],&regData);
                        dbg.print("regData:");
                        dbg.print(RCstr(_retCode));
                        dbg.print(":0x");
                        dbg.print(regData,HEX);
                        dbg.print(":");
                        dbg.println(regData,DEC);
                        CSselectDevice(DEVDISABLE);
                        break;
                    } 
                }
                break;
            case 'B':
                printIRMSVRMSZX(_testChannel); 
				break;
            case 'C':                       //Change active channel for ADE, switching, and metering
                _testChannel = getChannelID();    
                break;
            case 'D':                       //Initialize ckts[] to safe defaults
                for (int i = 0; i < NCIRCUITS; i++) {
                    c = &ckts[i];
                    CsetDefaults(c,i);
                }
                dbg.println("Defaults set. Don't forget to program! ('P')");
                break;
            case 'E':                       //Save data in ckts[] to EEPROM
                dbg.println("Saving to EEPROM.");
                for (int i =0; i < NCIRCUITS; i++) {
                    Csave(&ckts[i],&cktsSave[i]);
                }
                dbg.println(COMPLETESTR);
                break;
            case 'e':                       //Load circuit data from EEPROM
                dbg.println("Loading from EEPROM.");
                for (int i =0; i < NCIRCUITS; i++) {
                    Cload(&ckts[i],&cktsSave[i]);
                }
                dbg.println(COMPLETESTR);
                break;
            case 'F':                       //Test switch aggresively
                testSwitch(_testChannel);
                break;
            case 'f':                       //Quick diagnostic test of basic functionality
                testHardware();
                break;
            case 'L':                       //Run calibration routine on channel
                c = &(ckts[_testChannel]);
                calibrateCircuit(c);
                break;
            case 'l':                       //Read SD card info, quick sd card test
                printSDCardInfo();
                break;
            case 'P':                       //Program values in ckts[] to ADE
                for (int i = 0; i < NCIRCUITS; i++) {
                    c = &ckts[i];
                    Cprogram(c);
                    codes[i] = RCstr(_retCode);
                }
                printTableStrings(codes,NCIRCUITS);
                break;
            case 'p':                       //Print Circuit values
                c = &(ckts[_testChannel]);
                Cprint(&dbg,c);
                dbg.println();
                break;
            case 'R':                       //Hard Reset using watchdog timer
               wdt_enable((WDTO_4S));            
               dbg.println(" #resetting in 4s.");
               break;
            case 'S':                       //Toggle channel switch
                ID = getChannelID();        
                SWset(ID,!SWisOn(ID));
                break;
            case 's':                       //Display switch state
                displayEnabled(SWgetSwitchState());
                break;
            case'T':                        //Change reporting interval
                dbg.print(" #New Reporting Interval:");
                ifsuccess(CLgetInt(&dbg,&retVal)) {
                    reportInterval = retVal;
                }
                break;
            case 't':
                dbg.print(" #Reporting Interval:");
                dbg.println(reportInterval);
                break;
            case 'M':                       //Change Interaction Mode
                dbg.print(" #2 for meter mode, 1 for interactive mode:"); 
                ifsuccess(CLgetInt(&dbg,&retVal)) {
                    if ( retVal == INTERACTIVEMODE || retVal == METERMODE) {
                        mode = retVal;
                        dbg.println();
                        return;
                    }
                }
                dbg.println();
                dbg.println("Bad Input.");
                break;
            case 'm':                       //Meter but do not print
                Cmeasure(&ckts[_testChannel]);
                ifsuccess(_retCode) {
                    dbg.println("#Meter Completely Successful.");
                } else {
                    dbg.println("#Meter unsuccessful.");
                }
                RCreset();
                break;
            case 'o':                       //Wait for zero-crossing
                CSselectDevice(_testChannel);
                CLwaitForZX10VIRMS();
                CSselectDevice(DEVDISABLE);
                break;
            case 'x':                       //Wait for interrupt specified by interrupt mask
                dbg.println();
                dbg.println("Available interrupt masks:");
                for (int i =0; i < intListLen; i++){
                    dbg.print( intList[i]);
                    dbg.print(" ");
                }
                dbg.println();
                dbg.print("Enter interrupt mask name or \"mask\" "
                        "to enter a mask manually. " 
                        "Will wait for 4sec for interrupt to fire. $");
                CLgetString(&dbg,buff,sizeof(buff));
                if (!strcmp(buff, "mask")) {
                    dbg.print("Enter interrupt mask as a number. $");
                    CLgetInt(&dbg,&mask);
                } else {
                    mask=1;
                    for (int i =0; i < intListLen; i++){
                        if (!strcmp(buff, intList[i])) {
                            break;
                        }
                        mask <<= 1;
                    }
                }
                dbg.println();
                CSselectDevice(_testChannel);
                ADEwaitForInterrupt((int16_t)mask,4000);
                dbg.println(RCstr(_retCode));
                CSselectDevice(DEVDISABLE);
                break;
            case 'X':                       // Read WAVEFORM Data need to configure registers first!
                CSselectDevice(_testChannel);
                for (int i =0; i < 80; i++) {
                    ADEgetRegister(WAVEFORM,&regData);
                    dbg.print(regData);
                    dbg.println(" Note: Did you configure the MODE and Interrupt registers properly?");
                }
                CSselectDevice(DEVDISABLE);
                break;
            case 'z':                       // Print long-run test results
                testCircuitPrint();
                break;
            case 'Z':                       // Long-run test with metering 
                                            // at regular intervals 
                                            // and constant switching inbetween
                if (testIdx) {
                    dbg.print("Test Canceled");
                    testIdx = 0;
                    return;
                }

                dbg.println();
                dbg.print("Minutes to run test $");
                CLgetInt(&dbg,&runMin);
                dbg.println();
                dbg.print("Seconds delay between each experiment. $");
                CLgetInt(&dbg,&switchSec);
                if (switchSec < 0) {
                    switchSec = 0;
                }
                switchSec += 120/60;
                testIdx = runMin*60/switchSec;

                dbg.println();
                dbg.print("Total number of experiments is: ");
                dbg.print(testIdx);
                dbg.println();
                if (testIdx > RARAASIZE) {
                    dbg.print("Too many experiments to run. Make test shorter or test with a longer period between experiments.");
                    dbg.println();
                    testIdx = 0;
                    return;
                }

                //Select it
                CSselectDevice(_testChannel);

                //Configure meter
                ADEsetRegister(LINECYC,&linecycVal);
                ADEsetRegister(GAIN,&gain);
                ADEsetRegister(PHCAL,&phcal);
                ADEsetModeBit(CYCMODE,1);

                //Initialize storage area for results
                eeprom_update_block(&testIdx,&nRARAASave,sizeof(testIdx));
                for (int i=testIdx; i>0; i--) {
                    zeros[1] = i;
                    eeprom_update_block(zeros,RARAASave,sizeof(RARAASave[0]));
                }
                switchings = 0;
                dbg.print("Test started.");
                break;
            default:                        //Indicate received character
                int waiting = 128;             //Used to eat up junk that follows
                do {
                    dbg.println();
                    dbg.print("Not_Recognized:");
                    dbg.print(incoming,BIN);
                    dbg.print(":'");
                    dbg.print(incoming);
                    dbg.println("'");

                    if (dbg.available()) {
                        incoming = dbg.read();
                    }
                    waiting--;
                } while (dbg.available() && waiting > 0);
                break;
        }
    }

    setDbgLeds(0);
}

/** resets the test channel (input daughter board) to default parameters and sets the linecycle count up.
*/
void softSetup()
{
    int32_t data = 0;

    dbg.print("\n\n\rSetting Channel:");
    dbg.println(_testChannel,DEC);

    CSselectDevice(_testChannel); //start SPI comm with the test device channel

    //Disable Digital Integrator for _testChannel
    int8_t ch1os=0,enableBit=0;
    dbg.print("set CH1OS:");
    ADEsetCHXOS(1,&enableBit,&ch1os);
    dbg.println(RCstr(_retCode));
    dbg.print("get CH1OS:");
    ADEgetCHXOS(1,&enableBit,&ch1os);
    dbg.println(RCstr(_retCode));
    dbg.print("enabled: ");
    dbg.println(enableBit,BIN);
    dbg.print("offset: ");
    dbg.println(ch1os);

    //set the gain to 16 for channel _testChannel since the sensitivity appears to be 0.02157 V/Amp
    int32_t gainVal = 0x4;
    dbg.print("BIN GAIN (set,get):");
    ADEsetRegister(GAIN,&gainVal);
    dbg.print(RCstr(_retCode));
    dbg.print(",");
    ADEgetRegister(GAIN,&gainVal);
    dbg.print(RCstr(_retCode));
    dbg.print(":");
    dbg.println(gainVal,BIN);

    //NOTE*****  I am using zeros right now because we are going to up the gain and see if this is the same
    //Set the IRMSOS to 0d444 or 0x01BC. This is the measured offset value.
    int32_t iRmsOsVal = 0x0;//0x01BC;
    ADEsetRegister(IRMSOS,&iRmsOsVal);
    ADEgetRegister(IRMSOS,&iRmsOsVal);
    dbg.print("hex IRMSOS:");
    dbg.println(iRmsOsVal, HEX);

    //Set the VRMSOS to -0d549. This is the measured offset value.
    int32_t vRmsOsVal = 0x0;//0x07FF;//F800
    ADEsetRegister(VRMSOS,&vRmsOsVal);
    ADEgetRegister(VRMSOS,&vRmsOsVal);
    dbg.print("hex VRMSOS read from register:");
    dbg.println(vRmsOsVal, HEX);

    //set the number of cycles to wait before taking a reading
    int32_t linecycVal = 200;
    ADEsetRegister(LINECYC,&linecycVal);
    ADEgetRegister(LINECYC,&linecycVal);
    dbg.print("int linecycVal:");
    dbg.println(linecycVal);

    //read and set the CYCMODE bit on the MODE register
    int32_t modeReg = 0;
    ADEgetRegister(MODE,&modeReg);
    dbg.print("bin MODE register before setting CYCMODE:");
    dbg.println(modeReg, BIN);
    modeReg |= CYCMODE;     //set the line cycle accumulation mode bit
    ADEsetRegister(MODE,&modeReg);
    ADEgetRegister(MODE,&modeReg);
    dbg.print("bin MODE register after setting CYCMODE:");
    dbg.println(modeReg, BIN);

    //reset the Interrupt status register
    ADEgetRegister(RSTSTATUS, &data);
    dbg.print("bin Interrupt Status Register:");
    dbg.println(data, BIN);

    CSselectDevice(DEVDISABLE); //end SPI comm with the selected device    
}

void displayChannelInfo() 
{
    int32_t val;
    uint32_t iRMS = 0;
    uint32_t vRMS = 0;
    uint32_t lineAccAppEnergy = 0;
    uint32_t lineAccActiveEnergy = 0;
    int32_t interruptStatus = 0;
    uint32_t iRMSSlope = 164;
    uint32_t vRMSSlope = 4700;
    uint32_t appEnergyDiv = 5;
    uint32_t energyJoules = 0;

    //Select the Device
    CSselectDevice(_testChannel);

    //Read and clear the Interrupt Status Register
    ADEgetRegister(RSTSTATUS, &interruptStatus);

    if (0 /*loopCounter%4096*/ ){
        dbg.print("bin Interrupt Status Register:");
        dbg.println(interruptStatus, BIN);
    }   //endif

    //if the CYCEND bit of the Interrupt Status Registers is flagged
    dbg.print("\n\n\r");
    dbg.print("Waiting for next cycle: ");
    ADEwaitForInterrupt(CYCEND,4000);
    dbg.println(RCstr(_retCode));

    ifsuccess(_retCode) {
        setDbgLeds(GYRPAT);

        dbg.print("_testChannel:");
        dbg.println(_testChannel,DEC);

        dbg.print("bin Interrupt Status Register:");
        dbg.println(interruptStatus, BIN);

        //IRMS SECTION
        dbg.print("IRMS:");
        ADEgetRegister(IRMS,&val);
        dbg.println( RCstr(_retCode) );
        dbg.print("Counts:");
        dbg.println(val);
        dbg.print("mAmps:");
        iRMS = val/iRMSSlope;//data*1000/40172/4;
        dbg.println(iRMS);

        //VRMS SECTION
        dbg.print("VRMS:");
        ADEgetRegister(VRMS,&val);
        dbg.println(RCstr(_retCode));
        dbg.print("Counts:");
        dbg.println(val);
        vRMS = val/vRMSSlope; //old value:9142
        dbg.print("Volts:");
        dbg.println(vRMS);


        //APPARENT ENERGY SECTION
        ADEgetRegister(LVAENERGY,&val);
        dbg.print("int Line Cycle Apparent Energy after 200 half-cycles:");
        dbg.println(val);
        energyJoules = val*2014/10000;
        dbg.print("Apparent Energy in Joules over the past 2 seconds:");
        dbg.println(energyJoules);
        dbg.print("Calculated apparent power usage:");
        dbg.println(energyJoules/2);

        //ACTIVE ENERGY SECTION
        ADEgetRegister(LAENERGY,&val);
        ifsuccess(_retCode) {
            dbg.print("int Line Cycle Active Energy after 200 half-cycles:");
            dbg.println(val);
        } else {
            dbg.println("Line Cycle Active Energy read failed.");
        }// end ifsuccess
    } //end ifsuccess

    CSselectDevice(DEVDISABLE);
}

int8_t getChannelID() 
{
    int32_t ID = -1;
    while (ID == -1) {
        dbg.print("Waiting for ID (0-20):");
        ifnsuccess(CLgetInt(&dbg,&ID)) ID = -1;
        dbg.println();
        if (ID < 0 || 20 < ID ) {
            dbg.print("Incorrect ID:");
            dbg.println(ID,DEC);
            ID = -1;
        } else {
            dbg.println((int8_t)ID,DEC);
        }
    }
    return (int8_t)ID;
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

/** 
  Quickly turns on all circuits.
  Then turns off all of them as fast as possible except for MAINS.
  Then tries to communicate with the ADEs.
 */
void testHardware() 
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

/** 
  Lists the state of the circuit switches.
 */
void displayEnabled(const int8_t enabledC[NSWITCHES])
{
    dbg.println("Enabled Channels:");
    for (int i =0; i < NSWITCHES; i++) {
        dbg.print(i);
        dbg.print(":");
        dbg.print(enabledC[i],DEC);
        if (i%4 == 3) {
            dbg.println();
        } else {
            dbg.print('\t');
        }
    }
    dbg.println();
}

void printTableStrings(const char *strs[], int8_t len)
{
    for (int i=0; i < len; i++) {
        dbg.print(i); 
        dbg.print(':'); 
        dbg.print(strs[i]);
        if (i%4 != 3) {
            dbg.print('\t');
        } else {
            dbg.println();
        }
    }
}

void printIRMSVRMSZX( int channel ) {
    int32_t VRMSdata = 0;
    int32_t IRMSdata = 0;
    int32_t ZXstatus= 0;
    int32_t Istatus= 0;
    int32_t Vstatus= 0;

    CSselectDevice(channel);

    dbg.println("IRMS and VRMS after zero-crossing: ");
    //set interrrupt enable for zero crossing: ADDR 0x0A = 0x0010
    ADEsetIrqEnBit(ZX,true);
    //reset the interrupt status: Read Reg ADDR 0x0C
    ADEgetRegister(RSTSTATUS,&ZXstatus);
    //ZXstatus is chosen arbitrarily it gets erased later on
    //wait for Interrupt for 20 millisecs
    ADEwaitForInterrupt(ZX,20);
    ZXstatus = _retCode;
    //read VRMS
    ADEgetRegister(IRMS,&VRMSdata);
    Istatus = _retCode;
    //read IRMS
    ADEgetRegister(VRMS,&IRMSdata);
    Vstatus = _retCode;

    ifnsuccess(ZXstatus== TIMEOUT) {
        dbg.print("We didn't get the ZX interrupt: ");
        dbg.print(RCstr(ZXstatus));
    }

    dbg.print("IRMS:");
    dbg.print(RCstr(Istatus));
    dbg.print(":0x");
    dbg.print(IRMSdata,HEX);
    dbg.print(":");
    dbg.println(IRMSdata,DEC);

    dbg.print("VRMS:");
    dbg.print(RCstr(Vstatus));
    dbg.print(":0x");
    dbg.print(VRMSdata,HEX);
    dbg.print(":");
    dbg.println(VRMSdata,DEC);				

    CSselectDevice(DEVDISABLE);
}

void printSDCardInfo() 
{
    sd_raw_info sdinfo;
    sdinfo.capacity =0;
    sdinfo.manufacturer=0;
    sdinfo.manufacturing_year=0;
    
    sd_raw_get_info(&sdinfo);
    dbg.print("Inserted:");
    dbg.println((bool)sd_raw_available());
    dbg.print("Write Protected:");
    dbg.println((bool)sd_raw_locked());

    dbg.print("Capacity:");
    dbg.println((uint32_t)sdinfo.capacity);
    dbg.print("Manufacturer:");
    dbg.println((int16_t)sdinfo.manufacturer);
    dbg.print("Manufacturing Year:");
    dbg.println((int16_t)sdinfo.manufacturing_year);
}
