#include <avr/eeprom.h>

#include "ReturnCode/returncode.h"
#include "ADE7753/ADE7753.h"
#include "Select/select.h"
#include "Switches/switches.h"
#include "circuit.h"
#include "arduino/HardwareSerial.h"

#define dbg Serial
#define max(X,Y) ((X)>=(Y))?(X):(Y)
#define ERRCHECKRETURN(Cptr) if (_shouldReturn(Cptr)) return;

// TODO Have fault detection code check interrupts for sag detection and frequency variation
// ipeak is RMS or what basically should I multiple by root2?
// TODO make this a class separated from the implementation of the Meter.
// TODO reset shouldn't be aware of the topology of the daughterboards and switch state.

int8_t _shouldReturn(Circuit *c) 
{
    ifnsuccess(_retCode) {            
        if (_retCode == COMMERR) {        
            c->status |= COMM;            
            CSselectDevice(DEVDISABLE);    
            return true;                        
        } else if (_retCode == TIMEOUT) {
            c->status |= TIME;
        } else {                        
            CSselectDevice(DEVDISABLE);    
            return true;                        
        }                                
    }
    return false;
}

/**
 * Waits 1.5 tmes as long for functions which depend on the AC period.
 * */
uint16_t CwaitTime(Circuit *c) 
{
    int16_t cycles = c->halfCyclesSample/2 +1;
    uint16_t duration_ms = (uint16_t)((c->periodus/1000)*cycles);
    /*Wait at least 1.5 times the amount of time it takes for halfCycleSample halfCycles to occur*/
    return duration_ms + duration_ms/2; // At worst this is 65ms*cycles
}

/**
 * Clears circuit interrupts
 * */
void Cclear(Circuit *c) 
{
    int32_t regData;
    RCreset();
    CSselectDevice(c->circuitID);                       ERRCHECKRETURN(c);
    //Check for presence and clears the interrupt register
    //Comm errors are stored in c->status
    c->status=0;
    ADEgetRegister(RSTSTATUS,&regData);                 ERRCHECKRETURN(c);
    CSselectDevice(DEVDISABLE);                       ERRCHECKRETURN(c);
}

/** 
  Updates circuit measured parameters
  @warning As a prerequisite Cclear should be called first.
  @warning a communications error may leave Circuit *c in an inconsisent state.
  @warning The completion time of this function is dependent on the frequency of the line as well as halfCyclesSample. At worst the function will take one minute to return if halfCyclesSample is 1400 and the frequency drops below 40hz.
  @return ARGVALUEERR if the circuitID is invalid
  @return COMMERR if there is a communications error or the ADE is not detected
*/
void Cmeasure(Circuit *c)
{
    int32_t regData;
    int8_t timeout = false;
    RCreset();
    CSselectDevice(c->circuitID);                       ERRCHECKRETURN(c);
    delay(1);

    //Check for presence 
    ADEgetRegister(STATUS,&regData);                 ERRCHECKRETURN(c);
    c->status = (0x0000FFFF&regData);

    //Start measuring
    ADEgetRegister(PERIOD,&regData);                    ERRCHECKRETURN(c);
    c->periodus = periodTous(regData);
    dbg.print("period:");
    dbg.println(c->periodus);
    uint16_t waitTime = CwaitTime(c);
    dbg.print("waitTime:");
    dbg.println(waitTime);
    ADEwaitForInterrupt(CYCEND,waitTime);               ERRCHECKRETURN(c);
    //The failure may have occured because there was no interrupt
    if (_retCode == TIMEOUT) {
        timeout = true;
    }

    if (!timeout) {
        //Apparent power or Volt Amps
        ADEgetRegister(LVAENERGY,&regData);             ERRCHECKRETURN(c);
        c->VA = regData*c->VAEslope/2;// TODO assuming 2 seconds/(c->halfCyclesSample/2.0*c->periodus/1000000);  //Watts

        //Active power or watts
        ADEgetRegister(LAENERGY,&regData);              ERRCHECKRETURN(c);
        c->W = regData*c->Wslope/2;//TODO Assuming 2 seconds///(c->halfCyclesSample/2.0*c->periodus/1000000); //The denominator is the actual time in seconds

        //IRMS
        ADEgetRegister(IRMS,&regData);                  ERRCHECKRETURN(c);
        c->IRMS = regData*c->IRMSslope;

        //VRMS
        ADEgetRegister(VRMS,&regData);                  ERRCHECKRETURN(c);
        c->VRMS= regData*c->VRMSslope;

        //Apparent energy in millijoules accumulated since last query
        ADEgetRegister(RVAENERGY,&regData);             ERRCHECKRETURN(c);
        c->VAEnergy = regData*c->VAEslope;

        //Actve energy in millijoules accumulated since last query
        ADEgetRegister(RAENERGY,&regData);              ERRCHECKRETURN(c);
        c->WEnergy = regData*c->Wslope/1000;

        //Current and Voltage Peaks TODO in whatever units
        ADEgetRegister(RSTIPEAK,&regData);              ERRCHECKRETURN(c);
        c->ipeak = regData;

        ADEgetRegister(RSTVPEAK,&regData);              ERRCHECKRETURN(c);
        c->vpeak = regData;

        //Power Factor PF
        if (c->VAEnergy != 0){ 
            c->PF = (uint16_t)(((((uint64_t)c->WEnergy<<16)-c->WEnergy)-c->WEnergy)/c->VAEnergy);
        } else {
            c->PF = 65535;
        }
    } //end if (!timeout)

    CSselectDevice(DEVDISABLE);

    // Since the above get register errors would hide the TIMEOUT
    if (timeout) {
        _retCode = TIMEOUT;
    }
}

void Cprogram(Circuit *c)
{
    int32_t regData;

    RCreset();
    CSselectDevice(c->circuitID);                       ERRCHECKRETURN(c);

    ADEreset();

    //If there is some non-zero sag duration cycle set it
    if (c->sagDurationCycles > 0) { 
        regData = c->sagDurationCycles + 1;
        ADEsetModeBit(DISSAG,false);                    ERRCHECKRETURN(c);
        ADEsetRegister(SAGCYC,&regData);                ERRCHECKRETURN(c);
    } else {
        ADEsetModeBit(DISSAG,true);                     ERRCHECKRETURN(c);
    }
    regData = c->phcal;                                 ERRCHECKRETURN(c);
    ADEsetRegister(PHCAL,&regData);

    ADEsetCHXOS(1,&c->chIint,&c->chIos);                ERRCHECKRETURN(c);
    regData = c->IRMSoffset;
    ADEsetRegister(IRMSOS, &regData);                   ERRCHECKRETURN(c);
    //since this is channel 2 c->chIint is ignored
    ADEsetCHXOS(2,&c->chIint,&c->chVos);                ERRCHECKRETURN(c);
    regData = c->VRMSoffset;
    ADEsetRegister(VRMSOS, &regData);                   ERRCHECKRETURN(c);
    //If there is some non-zero cycle sample time set CYCMODE appropriately
    if (c->halfCyclesSample> 0) {
        regData = c->halfCyclesSample;
        ADEsetRegister(LINECYC,&regData);               ERRCHECKRETURN(c);
        ADEsetModeBit(CYCMODE,true);                    ERRCHECKRETURN(c);
    } else {
        ADEsetModeBit(CYCMODE,false);                   ERRCHECKRETURN(c);
    }

    //Set gains and scale
    regData =  (c->chVgainExp<<5) | (c->chVscale<<3) | c->chIgainExp;
    ADEsetRegister(GAIN,&regData);                      ERRCHECKRETURN(c);
    
    CSselectDevice(DEVDISABLE);
}

/**
 * If switch state is not already in the desired state, wait for a zero-crossing or 50ms and then switches.
 * Leaves _retcode as is.
 * */
void CsetOn(Circuit *c, int8_t on) 
{
    int8_t code = _retCode;
    int8_t durationms = c->periodus/1000;
    if (durationms > 50|| durationms < 10) durationms = 50;
    if (CisOn(c) != on) {
        CwaitForZX10(durationms);
    }
    _retCode = code;
    SWset(c->circuitID,on);
}

/**
 * Returns switch state.
 * */
int8_t CisOn(Circuit *c) 
{
    return SWisOn(c->circuitID);
}

/**
 *  Loads circuit data from the EEPROM into memory. This data can now be used to program the registers.
 * */
void Cload(Circuit *c, Circuit* addrEEPROM)
{
    eeprom_read_block(c,(uint8_t*)addrEEPROM,sizeof(Circuit));
}

/**
 *  Save circuit data from the memory into EEPROM.
 * */
void Csave(Circuit *c, Circuit* addrEEPROM) 
{
    eeprom_update_block(c,(uint8_t*)addrEEPROM,sizeof(Circuit));
}

/** 
    Reasonable default values for Circuit.
    Igain and Vgain are 0 assuming the maximum input 
    current is 15A with a .03Ohm shunt and 
    the maximum input voltage is 340V for 240RMS.
    vscale is 0 for a full scale of .5V
    @warning Does not program the ADE7753s
  */
void CsetDefaults(Circuit *c, int8_t circuitID) 
{
    c->circuitID = circuitID;

    /** Measurement Configuration Parameters */
    c->halfCyclesSample = 120;
    c->phcal = 11;          //Ox0B

    /** Current Calibration Parameters  */
    c->chIint = false;      //false unless a current loop or some dI/dt sensor is used.
    c->chIos = 0;           //Max +-31d
    c->chIgainExp = 0;      // .03Ohm to achieve max disipation of CS resistors Imax = 11.5
    c->IRMSoffset = -2048;  //-2048;//0x01BC;
    c->IRMSslope = .00468;  /** in mA/Counts */

    /** Voltage Calibration Parameters */
    c->chVos = 1;           // Max +-31d
    c->chVgainExp = 0;      //Version 5 is 0 as 380V -> 10:1 -> .380V at ADE
    c->chVscale = 0;
    c->VRMSoffset = -2048;  //-2048;//0x07FF;
    c->VRMSslope = 2.18;//.1069; /** in mV/Counts */

    /** Power Calibration Parameters */
    c->VAEslope = 75300;//34.2760;//2014/10000.0; mJ/Counts
    c->VAoffset = 0;// TODO not used yet
    c->Wslope= 31050; // TODO not used yet mJ/Counts for watts
    c->Woffset = 0;// TODO not used yet

    /** Software Saftey Parameters */
    c->sagDurationCycles = 10;
    c->minVSag = 100;
    c->VAPowerMax = 2000;
    c->ipeakMax = 16000;
    c->vpeakMax = 400;

    // Current and Voltage
    // Measured
    c->IRMS = 0;
    c->VRMS = 0;
    c->periodus = 50000;
    c->VA = 0;
    c->W = 0;
    c->PF = 1234;// Is a value from 0 to 2^16-1
    c->VAEnergy = 0;
    c->WEnergy = 0;
    c->ipeak = 123;
    c->vpeak = 123;

    c->status = 0;
}

/**
 * Prints all circuit values from memory and the current ADE settings for the circuit. 
 * @warning These values are not correlated until they have been programmed.
 * */
void Cprint(HardwareSerial *ser, Circuit *c) 
{

    ser->print("#CIRCUIT");
    ser->print("circuitID:"); ser->print(c->circuitID);
    ser->print("\thalfCyclesSample:"); ser->print(c->halfCyclesSample);
    ser->print("\tphcal:"); ser->println(c->phcal);

    ser->print("chIint:"); ser->print(c->chIint);
    ser->print("\tchIOS:"); ser->print(c->chIos);
    ser->print("\tchIgainExp:"); ser->println(c->chIgainExp);

    ser->print("IRMSOS:"); ser->print(c->IRMSoffset);
    ser->print("\tIRMS slope:"); ser->print(c->IRMSslope);
    ser->print("\tchVOS:"); ser->println(c->chVos);

    ser->print("chIgainExp:"); ser->print(c->chVgainExp);
    ser->print("\tchVscale:"); ser->print(c->chVscale);
    ser->print("\tVRMSOS:"); ser->println(c->VRMSoffset);

    ser->print("VRMS slope:"); ser->print(c->VRMSslope);
    ser->print("\tVAE slope:"); ser->print(c->VAEslope);
    ser->print("\tVA OS:"); ser->println(c->VAoffset);

    ser->print("W OS:"); ser->print(c->VAoffset);
    ser->print("\tW slope:"); ser->println(c->Wslope);

    ser->print("IRMS:"); ser->print(c->IRMS);
    ser->print("\tVRMS:"); ser->print(c->VRMS);
    ser->print("\tPeriod:"); ser->println(c->periodus);
    ser->print("VA:"); ser->print(c->VA);
    ser->print("\tW:"); ser->print(c->W);
    ser->print("\tPF:"); ser->println(c->PF);
    ser->print("VA Energy:"); ser->print(c->VAEnergy);
    ser->print("\tW Energy:"); ser->print(c->WEnergy);
    ser->print("\tipeak:"); ser->println(c->ipeak);
    ser->print("vpeak:"); ser->println(c->vpeak);

    RCreset();
    CSselectDevice(c->circuitID);
    ser->println("#ADE");
    for (const ADEReg** reg = &regList[0]; reg < &(regList[regListSize/sizeof(*reg)-1]);reg++) {
        int32_t regData = 0;
        ser->print((**reg).name); ser->print("& "); 
        ADEgetRegister(**reg,&regData);
        ifsuccess(_retCode) {
            ser->print(":0x");
            ser->print(regData,HEX);
            ser->print(":");
            ser->print(regData,DEC);
        } else {
            ser->println("FAILURE");
        }
        ser->println();
    }
    CSselectDevice(DEVDISABLE);
}

void CprintMeas(HardwareSerial *ser, Circuit *c)
{
    ser->print(c->circuitID,DEC);
    ser->print(",");
    ser->print(CisOn(c),DEC);
    ser->print(",");
    ser->print(c->VRMS,DEC);
    ser->print(",");
    ser->print(c->IRMS,DEC);
    ser->print(",");
    ser->print(c->vpeak,DEC);
    ser->print(",");
    ser->print(c->ipeak,DEC);
    ser->print(",");
    ser->print(c->periodus,DEC);
    ser->print(",");
    ser->print(c->VA,DEC);
    ser->print(",");
    ser->print(c->W,DEC);
    ser->print(",");
    ser->print(c->VAEnergy,DEC);
    ser->print(",");
    ser->print(c->WEnergy,DEC);
    ser->print(",");
    ser->print(c->PF,DEC);
    ser->print(",");
    // TODO VA ACCUM
    ser->print(0);
    ser->print(",");
    // TODO W ACCUM
    ser->print(0);
}

/** Strobe the CS pin on the current ADE.
 *
 * */
int8_t Cstrobe(Circuit *c) 
{
    int8_t comms =0;
    //Try strobing the CS pin
    CSselectDevice(c->circuitID);
    delay(1);
    CSselectDevice(DEVDISABLE);
    delay(1);
    CSselectDevice(c->circuitID);
    delay(1);
    CSselectDevice(DEVDISABLE);
    delay(1);
    comms = CtestComms(c);
    return comms; 
}


/** 
    Attempts to reestablish communications with the ADE.
    1) The SS pin is strobed  if that works return 1.
    2) IF that fails the ADE is reset and reprogrammed.
    @returns 0 if unable to restore. 1 if communication was successfully\
        restored without reprogramming and perhaps without data loss.\
        2 if meter was reprogrammed with dataloss.
*/
int8_t CrestoreCommunications(Circuit *c)
{
    if (Cstrobe(c)){
        return 1;
    }
    //Destructive reset
    Creset(c);
    if (CtestComms(c)) {
        return 2;
    }

    return 0;
}

/** 
 *  Checks to see if communications are working with the ADE
 */
int8_t CtestComms(Circuit *c) 
{
    CSselectDevice(c->circuitID);
    //Get DIEREV. Guaranteed not to be zero
    int32_t regData = 0;
    ADEgetRegister(DIEREV, &regData);
    CSselectDevice(DEVDISABLE);
    return regData && success(_retCode);
}

/**
 *  Resets the ADEs on the daughterboard and erases all data on the Circuit.
 *  Usually you want to follow this up with a reprogram of floor(circuitID/2) 
 *  and floor(circuitID/2) +1 unless you want virgin ADEs.
 */
void Creset(Circuit *c) 
{
    CSreset(c->circuitID);
}


/**
 * Waveform readings need to be read once they are ready 
 * as indicated by the interrrupt register.
 * The ADE must be configured properly beforehand.
 *
 * This function will retry several times and make an 
 * attempt to restore communications without resetting the ADE.
 * */
int32_t Cwaveform(void* c) 
{
    Circuit* cir = (Circuit*)c;
    int32_t waveform=0;
    CSselectDevice(cir->circuitID);
    for (int i = 0;i<10;i++) {
        RCreset();
        ADEgetRegister(RSTSTATUS,&waveform);
        ifnsuccess(_retCode) {
            Cstrobe(cir);
            CSselectDevice(cir->circuitID);
            continue;
        }
        ADEwaitForInterrupt(WSMP,CwaitTime(cir));
        ADEgetRegister(WAVEFORM,&waveform);
        ifnsuccess(_retCode) {
            Cstrobe(cir);
            CSselectDevice(cir->circuitID);
            continue;
        }
    }
    CSselectDevice(DEVDISABLE);
    return waveform;
}


/**
 * IRMS readings need to be read after the zero-crossing
 * The ADE must be configured properly beforehand.
 *
 * This function will retry several times and make an 
 * attempt to restore communications without resetting the ADE.
 * */
int32_t  Cvrms(void* c) 
{
    Circuit *cir = (Circuit*)c;
    int32_t vrms = 0;
    CSselectDevice(cir->circuitID);
    for (int i = 0; i<10; i++) {
        CwaitForZX10(CwaitTime(cir));
        if (_retCode == TIMEOUT) { return 0; } 
        else if (_retCode == COMMERR) {
            Cstrobe(cir);
            CSselectDevice(cir->circuitID);
            continue;
        } else {
            ADEgetRegister(VRMS,&vrms);
            if (_retCode == COMMERR) {
                Cstrobe(cir);
                CSselectDevice(cir->circuitID);
                continue;
            } else {
                RCreset();
                break;
            }
        }
    }
    CSselectDevice(DEVDISABLE);
    return vrms;
}

/**
 * VRMS readings need to be read after the zero-crossing
 * The ADE must be configured properly beforehand.
 *
 * This function will retry several times and make an 
 * attempt to restore communications without resetting the ADE.
 * */
int32_t Cirms(void* c)
{
    Circuit *cir = (Circuit*)c;
    int32_t irms = 0;
    CSselectDevice(cir->circuitID);
    for (int i = 0; i<10; i++) {
        CwaitForZX10(CwaitTime(cir));
        if (_retCode == TIMEOUT) { return 0; } 
        else if (_retCode == COMMERR) {
            Cstrobe(cir);
            continue;
        } else {
            ADEgetRegister(IRMS,&irms);
            if (_retCode == COMMERR) {
                Cstrobe(cir);
                continue;
            } else {
                RCreset();
                break;
            }
        }
    }
    CSselectDevice(DEVDISABLE);
    return irms;
}

/**
	Waits for the ZX (zero crossing) flag to transition to 0.
	retCode is set to TIMEOUT if it is never seen or COMMERR if there was a communiations issue.
  */
void CwaitForZX10(int8_t waitTime) 
{
	int32_t regData;
    RCreset();
	ADEgetRegister(RSTSTATUS,&regData); //reset interrupt ZX is now 1
    ifnsuccess(_retCode){return;}
	ADEwaitForInterrupt(ZX0,waitTime);  //Wait for the 1 to change to 0
    ifnsuccess(_retCode){return;}
}
