/**
 *  \mainpage Telduino
 *
 *  \section Purpose
 *    This code runs on a custom avr board with hardware to control 20 relays and measurement circuits.
 *
 *  \section Implementation
 *  The code is a client to an embedded linux system that sends string commands over the serial port.
 *  These serial commands are executed by the telduino code and sent back to the linux box.
 */

#include <stdlib.h>
#include <errno.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#include <string.h>
#include <stdint.h>

//Helper functions
#include "ReturnCode/returncode.h"
#include "prescaler.h"
#include "arduino/WProgram.h"

//Metering Hardware
#include "SPI/SPI.h"
#include "DbgTel/DbgTel.h"
#include "ADE7753/ADE7753.h"
#include "Select/select.h"
#include "sd-reader/sd_raw.h"
#include "Switches/switches.h"

//Metering logic
#include "Circuit/circuit.h"
#include "Circuit/calibration.h"

#include "Strings/strings.h"

//definition of serial ports for debug, sheeva communication, and telit communication
#define debugPort Serial // Serial 0 is the one hooked to the ftdi chip
#define sheevaPort Serial2
#define telitPort Serial3

#define DEBUG_BAUD_RATE 9600
#define SHEEVA_BAUD_RATE 9600
#define TELIT_BAUD_RATE 9600
#define verbose 1
#define MAXLEN_PLUG_MESSAGE 160

#define VERSION ""


boolean msgWaitLock = false;
Circuit ckts[NCIRCUITS];
Circuit EEMEM cktsSave[NCIRCUITS];

int _testChannel = 1; //This is the input daughter board channel. This should only be changed by the user.

int32_t switchSec= 0;

#define RARAASIZE 300
int32_t testIdx = 0;//Present index into RARAASAVE counts down to 0
int32_t EEMEM RARAASave[RARAASIZE][3] = {0};
int32_t EEMEM nRARAASave = 0; //Total number of entries in RARAASave

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void setup();
void loop();
void softSetup();
void setupLVAMode(int icid, int32_t linecycVal);
void setupRVAMode(int icid);
void setupDefaultMode(int icid);
void jobReadLVA(int icid);
void jobReadRVA(int icid);
void displayChannelInfo(); 
void displayEnabled(const int8_t enabledC[NSWITCHES]);
int8_t getChannelID();
void testSwitch(int8_t swID);
void testHardware();
void parseBerkeley();
void parseColumbia();
String getValueForKey(String key, String commandString);
void get_val(char *s, char *key, char *val);
String getSMSText(String commandString);
void meter(String commandString);
void meter_test(char *s);
void modem(String commandString);
void readSheevaPort();
void readTelitPort();
void chooseDestination(String destination, String commandString);
void turnOnTelit();

void setup()
{
    setClockPrescaler(CLOCK_PRESCALER_2);    //prescale of 2 after startup prescale of 8. This ensures that the arduino is running at 8 MHz.

    // start up serial ports
    debugPort.begin(DEBUG_BAUD_RATE);		//Debug serial
    telitPort.begin(TELIT_BAUD_RATE);		//Telit serial
    sheevaPort.begin(SHEEVA_BAUD_RATE);

    // write startup message to debug port
    debugPort.write("\r\n\r\ntelduino power up\r\n");
    debugPort.write("last compilation\r\n");
    debugPort.write(__DATE__);
    debugPort.write("\r\n");
    debugPort.write(__TIME__);
    debugPort.write("\r\n");

    //turnOnTelit();				// set telit pin high

    initDbgTel();				//Blink leds
    initSelect();				//Select Circuit
    SWinit();         //Switches
    //TODO sd_raw_init() is hanging
    //sd_raw_init();				//SDCard
    SPI.begin();				//SPI BUT lets move this into the ADE7753.c

    //The mains is the last line, do not turn it off
    /*
	SWallOff();
    delay(10000);
    SWallOn();
    for (int i = 0; i < NSWITCHES; i++) {
        if (i == MAINS) continue;
        SWset(i,false);
    }
	*/
	
    //Load circuit data from EEPROM
    for (int i=0; i < NCIRCUITS; i++) {
        Cload(&ckts[i],&cktsSave[i]);
    }
} //end of setup section


void testCircuitPrint() {
    int32_t records = 0;
    int32_t RARAA[3] = {0};
    //Get number of records from first block
    eeprom_read_block(&records,&nRARAASave,sizeof(records));

    //Iterate over these records and print in CSV format
    debugPort.println();
    debugPort.print("ON: RAENERGY,OFF:RAENERGY,AENERGY");
    debugPort.println();
    for (int i = records-1; i >0 ; i--) {
        eeprom_read_block(RARAA,&RARAASave[i],sizeof(RARAA));
        debugPort.print(RARAA[0]);
        debugPort.print(",");
        debugPort.print(RARAA[1]);
        debugPort.print(",");
        debugPort.print(RARAA[2]);
        debugPort.println();
    }
}


int8_t blinkTime()
{
    //Check for comm errors
    ifnsuccess(_retCode) {
        //FAILURE
        for (int i=0; i < 10; i++) {
            setDbgLeds(GRPAT);
            delay(100);
            setDbgLeds(OFFPAT);
            delay(100);
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
            delay(100);
            setDbgLeds(GPAT);
            delay(100);
        }
        return true;
        RCreset();
    }
    return false;
}

void loop()
{    
    parseBerkeley();
    //parseColumbia();
} //end of main loop

/**
 *  Single character serial interface for interaction with telduino
 *  Capital letters are usually writes and lower case letters are usually reads
 *  
 *  A
 *  a
 *  x
 *  C
 *  S
 *  s
 *  t
 *  T
 *  R
 *  a
 *  
 */
void parseBerkeley() 
{
    debugPort.println();
    debugPort.print(_testChannel,DEC);
    debugPort.print(" $");
    while (debugPort.available() == 0 && testIdx == 0) {
        setDbgLeds(GPAT);
        for (int i=0; i < 100; i++) {
            if (debugPort.available() != 0) {
                break;
            }
            delay(10);
        }
        setDbgLeds(0);
        for (int i=0; i < 100; i++) {
            if (debugPort.available() != 0) {
                break;
            }
            delay(10);
        }
    }

    if (testIdx > 0) {
        uint32_t startTime = millis();
        uint32_t val = 0;
        int32_t RARAA[3] = {0};
        //Switch the channel On
        SWset(_testChannel,true);
        ADEwaitForInterrupt(CYCEND,1200);
        //Meter it 
        ADEreadInterrupt(CYCEND); //Clear interrupt
        ADEwaitForInterrupt(CYCEND,1200);
        ADEwaitForInterrupt(CYCEND,1200);
        if (blinkTime()){
            RARAA[0] = -2000;
        } else {
            ADEgetRegister(LAENERGY,&RARAA[0]);
        }
        if (blinkComm()) {
            RARAA[0] = -1000;
        }

        //Switch the channel Off
        SWset(_testChannel,false);
        //Meter it 
        delay(1000);
        ADEgetRegister(RAENERGY,&RARAA[1]);
        if (blinkComm()) {
            RARAA[1] = -1000;
        }
        
        //Read AENERGY
        ADEgetRegister(AENERGY,&RARAA[2]);
        if (blinkComm()) {
            RARAA[2] = -1000;
        }

        debugPort.print(RARAA[0]);
        debugPort.print(",");
        debugPort.print(RARAA[1]);
        debugPort.print(",");
        debugPort.print(RARAA[2]);

        //Write to EEPROM
        eeprom_update_block(RARAA,&RARAASave[testIdx],sizeof(RARAA));

        uint32_t duration = (millis()-startTime);
        if (duration > 1000*switchSec) {
            duration = 1000*switchSec;
        }
        delay(1000*switchSec - duration);
        if (testIdx == 1){
            debugPort.println();
            debugPort.print("Testing Complete.");
            debugPort.println();
        }
        testIdx -= 1;
    }
    // Look for incoming single character command on debugPort line
    // Capital letters denote write operations and lower case letters are reads
    if (debugPort.available() > 0) {
        char incoming = debugPort.read(); 

        if (incoming == 'z') {
            testCircuitPrint();
        } else if (incoming == 'Z') {
            int32_t zeros[3] = {0};
            if (testIdx) {
                debugPort.print("Test Canceled");
                testIdx = 0;
                return;
            }

            int32_t runMin = 0;
            debugPort.println();
            debugPort.print("Minutes to run test $");
            CLgetInt(&debugPort,&runMin);
            debugPort.println();
            debugPort.print("Seconds delay between each experiment. $");
            CLgetInt(&debugPort,&switchSec);
            if (switchSec < 0) {
                switchSec = 0;
            }
            switchSec += 120/60;
            testIdx = runMin*60/switchSec;

            debugPort.println();
            debugPort.print("Total number of experiments is: ");
            debugPort.print(testIdx);
            debugPort.println();
            if (testIdx > RARAASIZE) {
                debugPort.print("Too many experiments to run. Make test shorter or test with a longer period between experiments.");
                debugPort.println();
                testIdx = 0;
                return;
            }

            //Select it
            CSselectDevice(_testChannel);

            //Configure meter
            int32_t linecycVal = 120;
            int32_t gain = 0x24;
            int32_t phcal = 0x0B;
            ADEsetRegister(LINECYC,&linecycVal);
            ADEsetRegister(GAIN,&gain);
            ADEsetRegister(PHCAL,&phcal);
            ADEsetModeBit(CYCMODE,1);

            //Initialize storage area for results
            eeprom_update_block(&testIdx,&nRARAASave,sizeof(testIdx));
            for (int i=testIdx; i>0; i--) {
                zeros[2] = i;
                eeprom_update_block(zeros,RARAASave,sizeof(zeros));
            }
            debugPort.print("Test started.");

        } else if (incoming == 'A') {            //Write to ADE Register
            char buff[16] = {0};
            debugPort.print("Register to write $");
            CLgetString(&debugPort,buff,sizeof(buff));
            debugPort.println();

            int32_t regData = 0;
            for (int i=0; i < regListSize/sizeof(regList[0]); i++) {
                if (strcmp(regList[i]->name,buff) == 0){
                    CSselectDevice(_testChannel);
                    debugPort.print("Current regData:");
                    ADEgetRegister(*regList[i],&regData);
                    debugPort.print(RCstr(_retCode));
                    debugPort.print(":0x");
                    debugPort.print(regData,HEX);
                    debugPort.print(":");
                    debugPort.println(regData,BIN);

                    debugPort.print("Enter new regData:");
                    if(CLgetInt(&debugPort,&regData) == CANCELED) break;
                    debugPort.println();
                    ADEsetRegister(*regList[i],&regData);
                    debugPort.print(RCstr(_retCode));
                    debugPort.print(":0x");
                    debugPort.print(regData,HEX);
                    debugPort.print(":");
                    debugPort.println(regData,DEC);
                    CSselectDevice(DEVDISABLE);
                    break;
                } 
            }
        } else if (incoming == 'a') {        //Read ADE reg
            char buff[16] = {0};
            debugPort.print("Enter name of register to read:");
            CLgetString(&debugPort,buff,sizeof(buff));
            debugPort.println();

            //debugPort.print("(int32_t)(&),HEX:");
            //debugPort.println((int32_t)(&WAVEFORM),HEX);
            int32_t regData = 0;
            for (int i=0; i < regListSize/sizeof(regList[0]); i++) {
                if (strcmp(regList[i]->name,buff) == 0){
                    CSselectDevice(_testChannel);
                    ADEgetRegister(*regList[i],&regData);
                    debugPort.print("regData:");
                    debugPort.print(RCstr(_retCode));
                    debugPort.print(":0x");
                    debugPort.print(regData,HEX);
                    debugPort.print(":");
                    debugPort.println(regData,DEC);
                    CSselectDevice(DEVDISABLE);
                    break;
                } 
            }
        } else if (incoming == 'x') {
            CSselectDevice(_testChannel);
            CLwaitForZX10VIRMS();
            CSselectDevice(DEVDISABLE);
        } else if (incoming == 'C') {        //Change active channel
            _testChannel = getChannelID();    
        } else if (incoming == 'S') {        //Toggle channel circuit
            int8_t ID = getChannelID();        
            SWset(ID,!SWisOn(ID));
        } else if (incoming == 's') {        //Display switch state
            displayEnabled(SWgetSwitchState());    
        } else if (incoming == 't') {        //Test basic functionality
            testHardware();
        } else if (incoming == 'T') {        //Test switch aggresively
            testSwitch(_testChannel);
        } else if (incoming == 'R') {        //Hard Reset using watchdog timer
            wdt_enable((WDTO_4S));            
           debugPort.println("resetting in 4s.");
        } else if (incoming == 'O') {        //soft Reset using the Setup routine
            softSetup();                    //Set calibration values for ADE
        } else if (incoming == 'o') {        //Read channel using Achintya's code
            displayChannelInfo();
        } else if (incoming == 'P') {        //Program values in ckts[] to ADE
            for (int i = 0; i < NCIRCUITS; i++) {
                Circuit *c = &(ckts[i]);
                Cprogram(c);
                debugPort.print(i); 
                debugPort.print(':'); 
                debugPort.print(RCstr(_retCode));
                if (i%4 != 3) {
                    debugPort.print('\t');
                } else {
                    debugPort.println();
                }
            }
            debugPort.println();
        } else if(incoming == 'p') {        //Measure circuit values and print
            Circuit *c = &(ckts[_testChannel]);
            Cmeasure(c);
            debugPort.println(RCstr(_retCode));
            CprintMeas(&debugPort,c);
            Cprint(&debugPort,c);
            debugPort.println();
        } else if (incoming == 'L') {        //Run calibration routine on channel
            Circuit *c = &(ckts[_testChannel]);
            calibrateCircuit(c);
            //debugPort.println(RCstr(_retCode));
        } else if (incoming == 'D') {		//Initialize ckts[] to safe defaults
            for (int i = 0; i < NCIRCUITS; i++) {
                Circuit *c = &ckts[i];
                CsetDefaults(c,i);
            }
            debugPort.println("Defaults set. Don't forget to program! ('P')");
        } else if (incoming == 'E') {        //Save data in ckts[] to EEPROM
            debugPort.println("Saving to EEPROM.");
            for (int i =0; i < NCIRCUITS; i++) {
                Csave(&ckts[i],&cktsSave[i]);
            }
            debugPort.println(COMPLETESTR);
        } else if (incoming=='e') {           //Load circuit data from EEPROM
            debugPort.println("Loading from EEPROM.");
            for (int i =0; i < NCIRCUITS; i++) {
                Cload(&ckts[i],&cktsSave[i]);
            }
            debugPort.println(COMPLETESTR);
        } else if (incoming=='w') {            //Wait for interrupt specified by interrupt mask
            int32_t mask = 0;
            char buff[10] = {0};
            debugPort.println();
            debugPort.println("Available interrupt masks:");
            for (int i =0; i < intListLen; i++){
                debugPort.print( intList[i]);
                debugPort.print(" ");
            }
            debugPort.println();
            debugPort.print("Enter interrupt mask name or \"mask\" "
                    "to enter a mask manually. " 
                    "Will wait for 4sec for interrupt to fire. $");
            CLgetString(&debugPort,buff,sizeof(buff));
            if (!strcmp(buff, "mask")) {
                debugPort.print("Enter interrupt mask as a number. $");
                CLgetInt(&debugPort,&mask);
            } else {
                mask=1;
                for (int i =0; i < intListLen; i++){
                    if (!strcmp(buff, intList[i])) {
                        break;
                    }
                    mask <<= 1;
                }
            }
            debugPort.println();
            //debugPort.print("(int32_t)(&),HEX:");
            //debugPort.println((int32_t)(&WAVEFORM),HEX);
            CSselectDevice(_testChannel);
            ADEwaitForInterrupt((int16_t)mask,4000);
            debugPort.println(RCstr(_retCode));
            CSselectDevice(DEVDISABLE);
        } else if (incoming == 'W')     {
            CSselectDevice(_testChannel);
            int32_t regData;
            for (int i =0; i < 80; i++) {
                ADEgetRegister(WAVEFORM,&regData);
                debugPort.print(regData);
                debugPort.print(" ");
            }
            CSselectDevice(DEVDISABLE);
        }
        else {                                //Indicate received character
            int waiting = 2048;                //Used to eat up junk that follows
            debugPort.println();
            debugPort.print("Not_Recognized:");
            debugPort.print(incoming,BIN);
            debugPort.print(":'");
            debugPort.print(incoming);
            debugPort.println("'");
            while (debugPort.available() || waiting > 0) {
                if (debugPort.available()) {
                    incoming = debugPort.read();
                    debugPort.println();
                    debugPort.print("Not_Recognized:");
                    debugPort.print(incoming,BIN);
                    debugPort.print(":'");
                    debugPort.print(incoming);
                    debugPort.println("'");
                } else     waiting--;
            }
        }
    }


    setDbgLeds(0);
}

//resets the test channel (input daughter board) to default parameters and sets the linecycle count up.
void softSetup()
{
    int32_t data = 0;

    debugPort.print("\n\n\rSetting Channel:");
    debugPort.println(_testChannel,DEC);

    CSselectDevice(_testChannel); //start SPI comm with the test device channel

    //Disable Digital Integrator for _testChannel
    int8_t ch1os=0,enableBit=0;
    debugPort.print("set CH1OS:");
    ADEsetCHXOS(1,&enableBit,&ch1os);
    debugPort.println(RCstr(_retCode));
    debugPort.print("get CH1OS:");
    ADEgetCHXOS(1,&enableBit,&ch1os);
    debugPort.println(RCstr(_retCode));
    debugPort.print("enabled: ");
    debugPort.println(enableBit,BIN);
    debugPort.print("offset: ");
    debugPort.println(ch1os);

    //set the gain to 16 for channel _testChannel since the sensitivity appears to be 0.02157 V/Amp
    int32_t gainVal = 0x4;
    debugPort.print("BIN GAIN (set,get):");
    ADEsetRegister(GAIN,&gainVal);
    debugPort.print(RCstr(_retCode));
    debugPort.print(",");
    ADEgetRegister(GAIN,&gainVal);
    debugPort.print(RCstr(_retCode));
    debugPort.print(":");
    debugPort.println(gainVal,BIN);

    //NOTE*****  I am using zeros right now because we are going to up the gain and see if this is the same
    //Set the IRMSOS to 0d444 or 0x01BC. This is the measured offset value.
    int32_t iRmsOsVal = 0x0;//0x01BC;
    ADEsetRegister(IRMSOS,&iRmsOsVal);
    ADEgetRegister(IRMSOS,&iRmsOsVal);
    debugPort.print("hex IRMSOS:");
    debugPort.println(iRmsOsVal, HEX);

    //Set the VRMSOS to -0d549. This is the measured offset value.
    int32_t vRmsOsVal = 0x0;//0x07FF;//F800
    ADEsetRegister(VRMSOS,&vRmsOsVal);
    ADEgetRegister(VRMSOS,&vRmsOsVal);
    debugPort.print("hex VRMSOS read from register:");
    debugPort.println(vRmsOsVal, HEX);

    //set the number of cycles to wait before taking a reading
    int32_t linecycVal = 200;
    ADEsetRegister(LINECYC,&linecycVal);
    ADEgetRegister(LINECYC,&linecycVal);
    debugPort.print("int linecycVal:");
    debugPort.println(linecycVal);

    //read and set the CYCMODE bit on the MODE register
    int32_t modeReg = 0;
    ADEgetRegister(MODE,&modeReg);
    debugPort.print("bin MODE register before setting CYCMODE:");
    debugPort.println(modeReg, BIN);
    modeReg |= CYCMODE;     //set the line cycle accumulation mode bit
    ADEsetRegister(MODE,&modeReg);
    ADEgetRegister(MODE,&modeReg);
    debugPort.print("bin MODE register after setting CYCMODE:");
    debugPort.println(modeReg, BIN);

    //reset the Interrupt status register
    ADEgetRegister(RSTSTATUS, &data);
    debugPort.print("bin Interrupt Status Register:");
    debugPort.println(data, BIN);

    CSselectDevice(DEVDISABLE); //end SPI comm with the selected device    
}

void displayChannelInfo() {
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
        debugPort.print("bin Interrupt Status Register:");
        debugPort.println(interruptStatus, BIN);
    }   //endif

    //if the CYCEND bit of the Interrupt Status Registers is flagged
    debugPort.print("\n\n\r");
    debugPort.print("Waiting for next cycle: ");
    ADEwaitForInterrupt(CYCEND,4000);
    debugPort.println(RCstr(_retCode));

    ifsuccess(_retCode) {
        setDbgLeds(GYRPAT);

        debugPort.print("_testChannel:");
        debugPort.println(_testChannel,DEC);

        debugPort.print("bin Interrupt Status Register:");
        debugPort.println(interruptStatus, BIN);

        //IRMS SECTION
        debugPort.print("IRMS:");
        ADEgetRegister(IRMS,&val);
        debugPort.println( RCstr(_retCode) );
        debugPort.print("Counts:");
        debugPort.println(val);
        debugPort.print("mAmps:");
        iRMS = val/iRMSSlope;//data*1000/40172/4;
        debugPort.println(iRMS);

        //VRMS SECTION
        debugPort.print("VRMS:");
        ADEgetRegister(VRMS,&val);
        debugPort.println(RCstr(_retCode));
        debugPort.print("Counts:");
        debugPort.println(val);
        vRMS = val/vRMSSlope; //old value:9142
        debugPort.print("Volts:");
        debugPort.println(vRMS);


        //APPARENT ENERGY SECTION
        ADEgetRegister(LVAENERGY,&val);
        debugPort.print("int Line Cycle Apparent Energy after 200 half-cycles:");
        debugPort.println(val);
        energyJoules = val*2014/10000;
        debugPort.print("Apparent Energy in Joules over the past 2 seconds:");
        debugPort.println(energyJoules);
        debugPort.print("Calculated apparent power usage:");
        debugPort.println(energyJoules/2);

        //ACTIVE ENERGY SECTION
        ADEgetRegister(LAENERGY,&val);
        ifsuccess(_retCode) {
            debugPort.print("int Line Cycle Active Energy after 200 half-cycles:");
            debugPort.println(val);
        } else {
            debugPort.println("Line Cycle Active Energy read failed.");
        }// end ifsuccess
    } //end ifsuccess

    CSselectDevice(DEVDISABLE);
}

int8_t getChannelID() 
{
    int32_t ID = -1;
    while (ID == -1) {
        debugPort.print("Waiting for ID (0-20):");
        ifnsuccess(CLgetInt(&debugPort,&ID)) ID = -1;
        debugPort.println();
        if (ID < 0 || 20 < ID ) {
            debugPort.print("Incorrect ID:");
            debugPort.println(ID,DEC);
            ID = -1;
        } else {
            debugPort.println((int8_t)ID,DEC);
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
void testHardware() {
    int8_t enabledC[NSWITCHES] = {0};
    int32_t val;

    debugPort.print("\n\rTest switches\n\r");

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

        debugPort.print("Can communicate with channel ");
        debugPort.print(i,DEC);
        debugPort.print(": ");

        ADEgetRegister(DIEREV,&val);
        ifnsuccess(_retCode) {
            debugPort.print("NO-");
            debugPort.println(RCstr(_retCode));
        } else {
            debugPort.print("YES-DIEREV:");
            debugPort.println(val,DEC);
        }
        CSselectDevice(DEVDISABLE);
    }
}

/** 
  Lists the state of the circuit switches.
 */
void displayEnabled(const int8_t enabledC[NSWITCHES])
{
    debugPort.println("Enabled Channels:");
    for (int i =0; i < NSWITCHES; i++) {
        debugPort.print(i);
        debugPort.print(":");
        debugPort.print(enabledC[i],DEC);
        if (i%4 == 3) {
            debugPort.println();
        } else {
            debugPort.print('\t');
        }
    }
    debugPort.println();
}

//JR needed to make compiler happy
extern "C" {
    void __cxa_pure_virtual(void) 
    {
        while(1) {
            setDbgLeds(RPAT);
            delay(332);
            setDbgLeds(YPAT);
            delay(332);
            setDbgLeds(GPAT);
            delay(332);
        }
    }
}

/*  Disables the watchdog timer the first chance the AtMega gets as recommended
    by Atmel.
 */
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();
    return;     
}

/* Parses &-delimited 'key=val' pairs and stores
 * the value for 'key' in 'val'
 */
void get_val(char *s, char *key, char *val)
{
    char *substr, *eq, *p, c;
    int i;

    substr = strstr(s, key);
    if (substr != NULL) {
        eq = strchr(substr, '=');
        if (eq != NULL) {
            p = eq;
            p++; // skip separator
            i = 0;
            c = *p;
            while ((c != NULL) && (c != '\0') && (c != '&') && \
                    (c != '\n') && (c != '\r')) {
                val[i++] = *p++;
                c = *p;
            }
            val[i] = '\0';
        }
    }
}

void meter_test(char *s)
{
    char job[8], s_cid[8];
    int8_t cid;

    // get job
    get_val(s, "job", job);
    // get cid
    get_val(s, "cid", s_cid);
    cid = atoi(s_cid); // could use strtod

    if (verbose > 0) {
        debugPort.println();
        debugPort.println("entered void meter()");
        debugPort.print("executing job type:");
        debugPort.print(job);
        debugPort.print(", on circuit id:");
        debugPort.println(cid);
        debugPort.println();
    }

    if (!strncmp(job, "con", 3)) {
        debugPort.println("execute con job");
        SWset(cid,1);
        debugPort.print("switch ");
        debugPort.print(cid, DEC);
        if (SWisOn(cid)) {
            debugPort.println(" is on");
        } else {
            debugPort.println(" is off");
        }
    }
    else if (!strncmp(job, "coff", 4)) {
        debugPort.println("execute coff job");
        SWset(cid,0);
        debugPort.print("switch ");
        debugPort.print(cid, DEC);
        if (SWisOn(cid)) {
            debugPort.println(" is on");
        } else {
            debugPort.println(" is off");
        }
    }
    else if (!strncmp(job, "readRVA", 7)) {
        jobReadRVA(cid);
    }
    else if (!strncmp(job, "readLVA", 7)) {
        jobReadLVA(cid);
    }
    else if (!strncmp(job, "modeRVA", 7)) {
        setupRVAMode(cid);
    }
    else if (!strncmp(job, "modeLVA", 7)) {
        int32_t line_cycle = 1000;
        char s_line_cycle[8];
        get_val(s, "linecyc", s_line_cycle);
        line_cycle = atoi(s_line_cycle); // could use strtod
        setupLVAMode(cid, line_cycle);
    }
    else if (!strncmp(job, "modeDefault", 11)) {
        setupDefaultMode(cid);
    }
    else if (!strncmp(job, "c", 1)) {
        _testChannel = cid;
        displayChannelInfo();		
    }
    else if (!strncmp(job, "T", 1)) {
        testHardware();
    }
    else if (!strncmp(job, "R", 1)) {
        wdt_enable((WDTO_4S));
        debugPort.println("resetting in 4s.");
    }
}

/**
 *	this function reads the telitPort (Serial3) for incoming commands
 *	and returns them as String objects.
 */
void readTelitPort() {
    uint32_t startTime = millis();
    byte b;
    while (telitPort.available()) {
        if ((b = telitPort.read()) != -1) {
            debugPort.print(b);
            sheevaPort.print(b);
            if (b == '>') { // modem awaits the content of the sms 
                msgWaitLock = true;
                delay(100);
            }
        } 
    }
}

void chooseDestination(String destination, String commandString) {
    /**
     *	based on the value for the cmp key, this calls the function
     *	meter if cmp=mtr
     *	and
     *  modem if cmp=mdm
     */
    if (destination == "mtr") {
        meter(commandString);
    }
    else if (destination == "mdm") {
        modem(commandString);
    }
}

void turnOnTelit() {
    /**
     *	Pull telit on/off pin high for 3 seconds to start up telit modem
     */
    pinMode(22, OUTPUT);
    digitalWrite(22, HIGH);
    delay(3000);
    digitalWrite(22, LOW);
}
