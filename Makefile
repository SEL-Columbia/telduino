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
GCCFLAGS = -c -g -Os -w -ffunction-sections -fdata-sections -Ilibraries/GSM
G++FLAGS = -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -Ilibraries/GSM
VPATH = libraries/GSM
GRND_OBJECTS = gsm.o gsmSMS.o ioHelper.o serial2.o timer.o $(PROJECT).o

all : GRND program

GRND : $(GRND_OBJECTS)
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega1280 -o $(PROJECT).elf $(GRND_OBJECTS) -Llibraries/GSM -lm
	avr-objcopy -O ihex -R .eeprom $(PROJECT).elf $(PROJECT).hex
	#avrdude -patmega1280 -cusbtiny -Uflash:w:$(PROJECT).hex

%.o : %.c
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) $< -o$@

%.o : %.cpp 
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) $< -o$@

clean:
	rm *.o *.elf *.eep *.hex

program: $(PROJECT).hex
	avrdude -patmega1280 -cusbtiny -Uflash:w:$(PROJECT).hex
