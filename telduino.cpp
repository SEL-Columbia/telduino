#include "arduino/WProgram.h"

//JR-Cal
#include "ShiftRegister/ShiftRegister.h"
#include "Demux/Demux.h"
#include "SDRaw/sd_raw.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"

char ctrlz = 26;

void setup();
void loop();
void echo_serial3_serial0();

//JR needed to make compiler happy
extern "C" {
	void __cxa_pure_virtual(void) {
		while(1);
	}
} 


void setup() {
	
	Serial.begin(9600);
	Serial3.begin(9600);
    
	Serial.print("\r\n");
	Serial.print("\r\n");
	Serial.print("telduino start\r\n");
	
	Serial3.print("AT+CMGF=1\r\n");
	echo_serial3_serial0();

	Serial3.print("AT+CSQ\r\n");
	echo_serial3_serial0();	
	
	Serial.print("message start\r\n");
	Serial3.print("AT+CMGS=8323776861\r\n");
	echo_serial3_serial0();
	Serial3.print("message from columbia telduino ");
	Serial3.println(__TIME__);
	echo_serial3_serial0();
	Serial3.write(ctrlz);
	echo_serial3_serial0();
	
}

void loop() {
	Serial3.print("AT+CCLK?\r\n");
	echo_serial3_serial0();
	delay(5000);
}

void echo_serial3_serial0(){
	delay(250);
	while (Serial3.available()){
		Serial.print(Serial3.read(), BYTE);
	}	
}

