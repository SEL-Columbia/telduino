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
GCCFLAGS = -c -g -Os -w -std=c99 -ffunction-sections -fdata-sections -Ilibraries
G++FLAGS = -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -Ilibraries
VPATH = libraries/GSM libraries/arduino libraries/Demux libraries/ShiftRegister \
    libraries/SPI libraries/DbgTel libraries/Select \
    libraries/ADE7753 \
    libraries/sd-reader
# libraries/SDRaw 

OBJECT_FILES = gsm.o gsmSMS.o gsmGPRS.o gsmMaster.o ioHelper.o \
	pins_arduino.o WInterrupts.o wiring.o wiring_analog.o wiring_digital.o \
	wiring_pulse.o wiring_shift.o Demux.o ShiftRegister.o main.o \
	HardwareSerial.o Print.o WMath.o WString.o SPI.o ADE7753.o \
	DbgTel.o select.o \
    byteordering.o fat.o partition.o sd_raw.o $(PROJECT).o 

all : hex program

hex : $(OBJECT_FILES)
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega1280 -o $(PROJECT).elf $(OBJECT_FILES) -Llibraries -lm
	avr-objcopy -j .text -j .data -O ihex -R .eeprom $(PROJECT).elf $(PROJECT).hex

%.o : %.c
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) $< -o$@

%.o : %.cpp 
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) $< -o$@

clean:
	rm *.o *.elf *.hex

program: $(PROJECT).hex
#	avrdude -patmega1280 -cusbtiny -Uflash:w:$(PROJECT).hex
	avrdude -patmega1280 -c dragon_isp -P usb -Uflash:w:$(PROJECT).hex
