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
#include "arduino/wiring.h"

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
#include "testMode.h"

void setup()
{
    // prescale of 2 after startup prescale of 8. 
    // This ensures that the atmega is running at 8 MHz assuming a 16Mhz clock.
    setClockPrescaler(CLOCK_PRESCALER_2);    

    // Start up serial ports
    dbg.begin(DBG_BAUD_RATE);
    mdm.begin(MDM_BAUD_RATE);
    cpu.begin(CPU_BAUD_RATE);

    // Write startup message to debug port
    dbg.println("\r\n\r\ntelduino power up");
    dbg.println("last compilation");
    dbg.println(__DATE__);
    dbg.println(__TIME__);

    DbgTelInit();				// Blink leds
    //initSelect();				// Select Circuit done in sd_raw_init
    sd_raw_init();			//SDCard
    SPI.begin();				// SPI
    SWinit();                   // Switches

    // Load circuit data from EEPROM
    for (int i=0; i < NCIRCUITS; i++) {
        Cload(&ckts[i],&cktsSave[i]);
    }
    for (int i=0; i < NCIRCUITS; i++) {
        Cprogram(&ckts[i]);
    }
}

void loop()
{   
    switch (mode) {
        case METERMODE:
            meterMode();
            break;
        case INTERACTIVEMODE:
            parseBerkeley();
            break;
        case TESTMODE:
            testMode();
            break;
        default:
            dbg.println("Invalid Mode setting to interactive mode");
            mode = INTERACTIVEMODE;
            break;
    }
}

extern "C" 
{
    /** 
     *  Called if there is a pure virtual function called.
     * */
    void __cxa_pure_virtual(void) 
    {
        DbgLeds(RPAT);
        delay(332);
        DbgLeds(YPAT);
        delay(332);
        DbgLeds(GPAT);
        delay(332);
        dbg.println("Pure Virtual Function Call. Resetting");
        wdt_enable((WDTO_4S));
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


