#include <string.h>
#include "arduino/WProgram.h"
#include "Select/select.h"
#include "circuit.h"
#include "calibration.h"
#include "ReturnCode/returncode.h"

#define dbg Serial1
/**
    Calibrate circuit interactively using serial port.
  */
int8_t calibrateCircuit(Circuit *c)
{
	float VlowCkt,VlowMeas;
	float VhighCkt,VhighMeas;
	float IlowCkt,IlowMeas;
	float IhighCkt,IhighMeas;
	CSSelectDevice(c->circuitID);
	//Check to see if the circuit can be enabled.
	int8_t retCode; 
	ifnsuccess(retCode = Cenable(c,true)) {
		dbg.print("Circuit could not be enabled:");
		dbg.println(RCstr(retCode));
	}
	CsetOn(c,false);
	//Query user to place load for low V,high I measurement
	dbg.println("Calibrating low-voltage and high-current (.8A).");
	dbg.print("Please attach a low-voltage source (120VAC 50Hz) ");
	dbg.print("and a low resistance load (e.g. 150 Ohm) to circuit \'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and transmit a \'\r\' (ENTER) when done.");
	while (dbg.read() != '\r');
	CsetOn(c,true);

	dbg.println("Enter measured voltage. Transmit a \'\r\' (ENTER) when done:");
	if (CLgetFloat(&dbg,&VlowCkt) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported:");
	dbg.println(VlowCkt,DEC);
	dbg.println("Enter measured current. Transmit a \'\r\' (ENTER) when done:");
	if (CLgetFloat(&dbg,&IhighCkt) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported:");
	dbg.println(IhighCkt,DEC);

	CsetOn(c,false);
	//Query user to place load for high V,low I measurement
	dbg.println("Calibrating high-voltage and low-current (.8A).");
	dbg.print("Please attach a low-voltage source (240VAC 50Hz) ");
	dbg.print("and a low resistance load (e.g. 2.4 KOhm) to circuit \'");
	dbg.print(c->circuitID,DEC);
	dbg.print("\' and transmit a \'\r\' (ENTER) when done.");
	while (dbg.read() != '\r');
	CsetOn(c,true);

	dbg.println("Enter measured voltage. Transmit a \'\r\' (ENTER) when done:");
	if (CLgetFloat(&dbg,&VhighCkt) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported:");
	dbg.println(VhighCkt,DEC);
	dbg.println("Enter measured current. Transmit a \'\r\' (ENTER) when done:");
	if (CLgetFloat(&dbg,&IlowCkt) == CANCELED) {
		dbg.println("CANCELED");
		return CANCELED;
	}
	dbg.print("Reported:");
	dbg.println(IlowCkt,DEC);

	/*Need to derive:
	IRMSoffset
	IRMSSlope
	VRMSoffset
	VRMSSlope
	chVos
	VASlope
	  */
	CSSelectDevice(DEVDISABLE);
}

/**
    Calibrate circuit using another circuit as the reference.
	  */
void autoCalibrateCircuit(Circuit *c,const Circuit *reference)
{

}

int8_t CLgetString(HardwareSerial *ser,char *buff, size_t bSize)
{
	int i=0;
	do {
		while (ser->available() < 1);
		buff[i] = ser->read();
		//Handle backspace
		if (buff[i] == '\x7F' && i>=1) {
			buff[i] = '\0';
			ser->print('\b');
			ser->print(' ');
			ser->print('\b');
			i -= 1;
		} else if (buff[i]=='\x7F' && i<=0){
		} else {
			if (buff[i] == '\r') {
				break;
			}
			ser->print(buff[i]);
			i++;
		}
	} while (i < (bSize-2));
	if (i == bSize-2 && buff[i] != '\r') {
		return FAILURE;
	}
	buff[i] = '\0';
	return SUCCESS;
}
int8_t CLgetFloat(HardwareSerial *ser,float *f)
{
	char buff[64] = {'\0'};
	do {
		while(nsuccess(CLgetString(ser,buff,sizeof(buff)))) {
			ser->println("Buffer overflow: command too long.");
		}
		if (sscanf(buff,"%f",f)) {
			return SUCCESS;
		}
		if (!strcmp(buff,"cancel")) {
			return CANCELED;
		}
	} while(true);
}
int8_t CLgetInt(HardwareSerial *ser,int *d)
{
	char buff[64] = {'\0'};
	do {
		while(nsuccess(CLgetString(ser,buff,sizeof(buff)))) {
			ser->println("Buffer overflow: command too long.");
		}
		if (sscanf(buff,"%d",d)) {
			return SUCCESS;
		}
		if (!strcmp(buff,"cancel")) {
			return CANCELED;
		}
	} while(true);
}
