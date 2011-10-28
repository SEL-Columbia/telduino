/**
 *  \mainpage Telduino
 *
 *	\section Purpose
 *	This code runs on a custom avr board with hardware to control 20 relays and measurement circuits.
 *
 *	\section Implementation
 *  The code is a client to an embedded linux system that sends string commands over the serial port.
 *  These serial commands are executed by the telduino code and sent back to the linux box.
 */
 
/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */

#include "arduino/WProgram.h"
#include "prescaler.h"

#include <avr/wdt.h>
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();
	return;     
}

void setup();
void loop();

void setup() {                
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  setClockPrescaler(CLOCK_PRESCALER_2);	//prescale of 2 after startup prescale of 8. This ensures that the arduino is running at 8 MHz.
  	pinMode(5, OUTPUT);
	pinMode(2, OUTPUT);
}

void loop() {
  digitalWrite(5, HIGH);   // set the LED on
	digitalWrite(2, LOW);
  delay(1000);              // wait for a second
  digitalWrite(5, LOW);    // set the LED off
	digitalWrite(2, HIGH);
  delay(1000);              // wait for a second
}