# this is all library object code stuff
# -c assemble but do not link
# -g include debug symbols
# -Os optimize for size
# -w inhibit all warning messages
# -ffunction-sections and -fdata-sections are for optimization of code location
# -I adds directory to the head of the list to be searched for header files
# -o place the output in file

# $< is the macro for the source in %.o : %.c
# $^ is the name of all prerequisites
# $@ first name of target of rule
# $* is the stem of a rule math %.o matches foo.o $* is foo


#select the file to run. telduino is main, telduino_test is going to be the test routines
PROJECT = telduino
PROGRAMMER = usbtiny #dragon_jtag #stk500 #dragon_jtag #dragon_isp
PORT = usb
BITPERIOD = 10
#PROJECT = telduino_test
MCU = atmega1280
CLOCK = 8000000L
LIBS = libscanf_flt.a
GCCFLAGS = -c -Os -w -Wall -std=c99 -ffunction-sections -fdata-sections -Icore 
G++FLAGS = -c -Os -w -Wall -fno-exceptions -ffunction-sections -fdata-sections -Icore


#-Wa,aln=foo.s
#No Codesize optimizations
#GCCFLAGS = -c -g -w -std=c99 -Icore
#G++FLAGS = -c -g -w -fno-exceptions -Icore
VPATH = core/arduino \
    core/SPI core/DbgTel core/Select \
    core/ADE7753 core/Switches \
	core/ReturnCode \
	core/Circuit core/sd-reader core/Statistics \
	core app
#	core/SDRaw 
#core/GSM 

OBJECT_FILES =  pins_arduino.o WInterrupts.o wiring.o wiring_analog.o \
	wiring_digital.o main.o \
	HardwareSerial.o Print.o SPI.o ADE7753.o \
	DbgTel.o select.o switches.o returncode.o  circuit.o calibration.o \
    byteordering.o fat.o partition.o sd_raw.o statistics.o interactive.o \
	meterMode.o testMode.o cfg.o circuit_controller.o $(PROJECT).o 

#TARGETS
.PHONY : clean install programfuses readfuses docs saverom
.DEFAUL_GOAL := update
update: compile program
compile: $(PROJECT).hex
install: programfuses update
all: compile program programfuses readfuses docs

%.hex : $(OBJECT_FILES)
	@avr-gcc -Os -Wl,--gc-sections -mmcu=$(MCU) -o $(PROJECT).elf $(OBJECT_FILES) -Lcore -lm
	@avr-objcopy -j .text -j .data -O ihex -R .eeprom $(PROJECT).elf $(PROJECT).hex
	@echo 
	@avr-size -C --mcu=$(MCU) $(PROJECT).elf 

size : $(PROJECT).elf
	@avr-size -C --mcu=$(MCU) $<

%.o : %.c
	@avr-gcc $(GCCFLAGS) -mmcu=$(MCU) -DF_CPU=$(CLOCK) $< -o$@ 

%.o : %.cpp
	@avr-g++ $(G++FLAGS) -mmcu=$(MCU) -DF_CPU=$(CLOCK) $< -o$@ 

clean:
	@rm -f *.o *.elf *.hex
	@rm -rf html/

program: $(PROJECT).hex
    #avrdude -p$(MCU) -c $(PROGRAMMER) -Uflash:w:$(PROJECT).hex
    #Set B to 10 or higher if programming fails or intermittently fails
	#avrdude -p$(MCU) -c $(PROGRAMMER) -P usb -Uflash:w:$< -B$(BITPERIOD)
	avrdude -p$(MCU) -c $(PROGRAMMER) -P $(PORT) -Uflash:w:$< -B$(BITPERIOD)

programfuses: 
#	low fuses: set external *FULLSWING* oscillator; startup time=16K clk + 0 ms; BOD enabled; divide clock by 8 initially
#	high fuses: enabled on chip debugging, jtag enabled, on reset start at 0000
	avrdude -p$(MCU) -c $(PROGRAMMER) -P usb -U lfuse:w:0x57:m -U hfuse:w:0x11:m -U efuse:w:0xf5:m -B10

readfuses:
	@avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -U hfuse:r:high.txt:r -U lfuse:r:low.txt:r -U efuse:r:ext.txt:r
	@echo -n "low  :" && hexdump low.txt
	@echo -n "high :" && hexdump high.txt
	@echo -n "efuse:" && hexdump ext.txt
	@rm -f low.txt high.txt ext.txt

saverom:
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -U flash:r:$(PROJECT).read.bin:r
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -U flash:r:$(PROJECT).read.hex:i


docs :
	doxygen
stats:
	@find -iname  "*.c*"|tr "\n"  "\0"|wc -l --files0-from=-|\
		tail -n1 | awk '{print $$$$2,"lines.";}'
#testBoard:
#	sudo screen /dev/ttyUSB0 9600 logfile $(date +%Y-%b-%d-%m-%S) logfile flush 1
