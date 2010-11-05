#include "arduino/WProgram.h"

//JR-Cal
#include "SDRaw/sd_raw.h"
#include "ADE7753/ADE7753.h"
#include "DbgTel/DbgTel.h"
#include "Select/select.h"

char ctrlz = 26;

void setup();
void loop();

//JR needed to make compiler happy
extern "C" {
#include "ShiftRegister/ShiftRegister.h"
#include "Demux/Demux.h"
	void __cxa_pure_virtual(void) {
		setDbgLeds(RPAT);
		delay(500);
		setDbgLeds(YPAT);
		delay(500);
		setDbgLeds(GPAT);
		delay(500);
		while(1);
	}
} 

void setup()
{
    //Enable level shifters
    pinMode(37, OUTPUT);
    digitalWrite(37,HIGH);

    //Setup serial port
    Serial2.begin(9600);

    //Blink leds
    initDbgTel();

    //test Shift registers
    initShiftRegister();
    //test muxers
    initDemux();

    //test select
    initSelect();


    delay(100);
    uint8_t data = 0b0000100;
    /*Serial2.println(data,BYTE);
    Serial2.println(writeData(8,0x0F,&data));
    delay(100);*/
    //Serial2.println(readData(8,0x3F,&data));
    delay(100);
    //Serial2.println(data,BYTE);
    if (data){
    //    Serial2.println("Uber Alles");
    }
}

void loop(){
    
    Serial2.println("Start");
    setDbgLeds(GYRPAT);
    delay(500);
    setDbgLeds(0);
    delay(500);
    //setDbgLeds(GYPAT);


    
    /* Shift Reg.
     * 
    int8_t first1 = 20;
    setEnabled(true);
    for (int8_t i = first1; i < first1 + 1; i++){
        shiftBit(true);
        //shiftBit(false);
        //latch();
    }
    for(int i = 0; i < first1; i++){
        shiftBit(false);
    }
    latch();
    delay(1000);
    clearShiftRegister();
    latch();
    */
    /* Demux*/
    /*
    //muxSetEnabled(true);
    muxSelect(16);
    for (int i = 0; i < 21; i++){
        //muxSelect(i);
        //delay(1000);
    }*/

    /* Select*/
    /*
    selectSPIDevice(SDCARD);
    delay(1000);
    for(int i =0; i < 21; i++){
        selectSPIDevice(i);
        delay(500);
    }
    */


    //SD Card
    /*
    selectSPIDevice(SDCARD);
    
    struct sd_raw_info info = {0}; 
    sd_raw_get_info(&info);
    if(info.manufacturer | info.revision) {
        Serial2.println(info.manufacturer);
        Serial2.println(info.revision);
    } else { 
    }
    delay(1000);
    */

    /* */
    /* ADE*/
    //INIT SPI
    /*
    SPI.begin();
    delay(100);
    selectSPIDevice(20);
    delay(100);
    uint8_t data[3];
    int8_t out = readData(24,VRMS,data);
    Serial2.println(out);
    Serial2.println(static_cast<int>(data[0]));
    Serial2.println(static_cast<int>(data[1]));
    Serial2.println(static_cast<int>(data[2]));
    delay(1000);
    */
    /*
    int8_t PAT = 0;
    if(data[0]){
        PAT |= GRNPAT;
    }
    if(data[1]){
        PAT |= YELPAT;
    }
    if(data[2]){
        PAT |=REDPAT;
    }
    setDbgLeds(PAT);
    delay(10000);
	*/
    
    Serial2.println("Restart");
    Serial2.flush();
}
