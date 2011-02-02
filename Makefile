# this is all library object code stuff
# -c assemble but do not link
# -g include debug stuff
# -Os optimize for size
# -w inhibit all warning messages
# -ffunction-sections and -fdata-sections are for optimization of code location
# -I adds directory to the head of the list to be searched for header files
# -o place the output in file

# $< is the macro for the source in %.o : %.c
# $^
# $@ first name of target of rule
# $*

CLOCK = 8000000L
PROJECT = telduino
GCCFLAGS = -c -g -Os -w -Wall -std=c99 -ffunction-sections -fdata-sections -Ilibraries
G++FLAGS = -c -g -Os -w -Wall -fno-exceptions -ffunction-sections -fdata-sections -Ilibraries

#-Wa,aln=foo.s
#GCCFLAGS = -c -g -Os -w -std=c99 -Ilibraries
#G++FLAGS = -c -g -Os -w -fno-exceptions -Ilibraries
VPATH = libraries/arduino libraries/Demux libraries/ShiftRegister\
    libraries/SPI libraries/DbgTel libraries/Select \
    libraries/ADE7753 libraries/Switches \
	libraries/ReturnCode \
	libraries/Circuit libraries/sd-reader
#libraries/SDRaw 
#libraries/GSM 

OBJECT_FILES =  pins_arduino.o WInterrupts.o wiring.o wiring_analog.o wiring_digital.o \
	wiring_pulse.o wiring_shift.o demux.o shiftregister.o main.o \
	HardwareSerial.o Print.o WMath.o WString.o SPI.o ADE7753.o \
	DbgTel.o select.o switches.o returncode.o  circuit.o calibration.o\
    byteordering.o fat.o partition.o sd_raw.o $(PROJECT).o 
#gsm.o gsmSMS.o gsmGPRS.o gsmMaster.o ioHelper.o

all : hex program 

hex : $(OBJECT_FILES)
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega1280 -o $(PROJECT).elf $(OBJECT_FILES) -Llibraries -lm
	avr-objcopy -j .text -j .data -O ihex -R .eeprom $(PROJECT).elf $(PROJECT).hex
	avr-size -C --mcu=atmega1280 $(PROJECT).elf 

%.o : %.c
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) $< -o$@ 

%.o : %.cpp 
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) $< -o$@ 

clean:
	rm -f *.o *.elf *.hex

program: $(PROJECT).hex
#	avrdude -patmega1280 -cusbtiny -Uflash:w:$(PROJECT).hex
	avrdude -patmega1280 -c dragon_isp -P usb -Uflash:w:$(PROJECT).hex -B10

programfuses:
#	low fuses: set external *FULLSWING* oscillator; startup time=16K clk + 0 ms; BOD enabled; divide clock by 8 initially
	avrdude -patmega1280 -c dragon_isp -P usb -U lfuse:w:0x57:m -U hfuse:w:0x91:m -U efuse:w:0xf5:m -B10

readfuses:
	avrdude -patmega1280 -c dragon_isp -P usb -U hfuse:r:high.txt:r -U lfuse:r:low.txt:r -U efuse:r:ext.txt:r
	echo -n "low  :" && hexdump low.txt
	echo -n "high :" && hexdump high.txt
	echo -n "efuse:" && hexdump ext.txt
	rm -f low.txt high.txt ext.txt

