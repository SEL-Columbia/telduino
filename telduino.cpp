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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#include "telduino.h"

// Helper functions
#include "prescaler.h"
#include "arduino/WProgram.h"

// Metering Hardware
#include "SPI/SPI.h"
#include "DbgTel/DbgTel.h"
#include "Select/select.h"
#include "sd-reader/sd_raw.h"
#include "Switches/switches.h"

// Metering logic
#include "Circuit/circuit.h"

// Meter modes
#include "cfg.h"
#include "interactive.h"
#include "meterMode.h"

void setup()
{
    // prescale of 2 after startup prescale of 8. 
    // This ensures that the atmega is running at 8 MHz assuming a 16Mhz clock.
    setClockPrescaler(CLOCK_PRESCALER_2);    

    // Start up serial ports
    dbg.begin(DEBUG_BAUD_RATE);
    mdm.begin(TELIT_BAUD_RATE);
    cpu.begin(SHEEVA_BAUD_RATE);

    // Write startup message to debug port
    dbg.println("\r\n\r\ntelduino power up");
    dbg.println("last compilation");
    dbg.println(__DATE__);
    dbg.println(__TIME__);

    initDbgTel();				// Blink leds
    initSelect();				// Select Circuit
    SWinit();                   // Switches
    SPI.begin();				// SPI
    // TODO sd_raw_init() is hanging
    // sd_raw_init();			//SDCard

    // Load circuit data from EEPROM
    for (int i=0; i < NCIRCUITS; i++) {
        Cload(&ckts[i],&cktsSave[i]);
    }
}


void loop()
{   
    if (mode == METERMODE) {
        if (cpu.available()) {
            char c = cpu.read();
            if (buffCursor < (serBuffSize-1)) {
                serBuff[buffCursor] = c;
                if (c == '\r') {
                    serBuff[buffCursor] = '\0';
                    buffCursor = 0;
                    dbg.println(serBuff);
                    parseMeterMode(serBuff);
                }
            } else {
                //TODO handle overflow better
                buffCursor = 0;
            }
        }
        meterAuto();
    } else if (mode == INTERACTIVEMODE) {
        parseBerkeley();
    } else {
        dbg.println("Invalid Mode setting to interactive mode");
        mode = INTERACTIVEMODE;
    }
}

extern "C" 
{
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

/**  Disables the watchdog timer. Should be specified to be run as soon as possible
 *   according to atmel. This is done in the declaration for this function.
 */
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();
    return;     
}


