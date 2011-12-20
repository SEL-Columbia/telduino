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

#define verbose 1
#define MAXLEN_PLUG_MESSAGE 160
#define VERSION ""


boolean msgWaitLock = false;

void setupLVAMode(int icid, int32_t linecycVal);
void setupRVAMode(int icid);
void setupDefaultMode(int icid);
void jobReadLVA(int icid);
void jobReadRVA(int icid);
void parseColumbia();
String getValueForKey(String key, String commandString);
void get_val(char *s, char *key, char *val);
void meter(String commandString);
void meter_test(char *s);
void modem(String commandString);
void readSheevaPort();
void readTelitPort();
void chooseDestination(String destination, String commandString);
void turnOnTelit();

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
