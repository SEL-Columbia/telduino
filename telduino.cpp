#include "util/delay.h"
#include "avr/io.h"
#include "avr/signal.h"
#include "WProgram.h"

void setup();
void loop();
void setup() {
	
	Serial.begin(9600);
    Serial3.begin(9600);
    
    DDRB = 0x80;
	
	
}

void loop() {
	PORTB = 0x80;
	_delay_ms(1000);  
	PORTB = 0x00;
	_delay_ms(1000);
	
	Serial.write("AT\r\n");
	Serial3.write("AT\r\n");
	
}
