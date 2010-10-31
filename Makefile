# this is all library object code stuff
# -c assemble but do not link
# -g include debug stuff
# -Os optimize for size
# -w inhibit all warning messages
# -ffunction-sections and -fdata-sections are for optimization of code location
# -I adds directory to the head of the list to be searched for header files
# -o place the output in file

# atmel 1280 comes with a default clock prescaling of 8
CLOCK = 1000000L
PROJECT = telduino
GCCFLAGS = -c -g -Os -w -ffunction-sections -fdata-sections
G++FLAGS = -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections 

arduino: telduino.cpp
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/pins_arduino.c -obuild/pins_arduino.c.o
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/WInterrupts.c -obuild/WInterrupts.c.o
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/wiring.c -obuild/wiring.c.o
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/wiring_analog.c -obuild/wiring_analog.c.o
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/wiring_digital.c -obuild/wiring_digital.c.o
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/wiring_pulse.c -obuild/wiring_pulse.c.o
	avr-gcc $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/wiring_shift.c -obuild/wiring_shift.c.o
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/HardwareSerial.cpp -obuild/HardwareSerial.cpp.o
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/main.cpp -obuild/main.cpp.o
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/Print.cpp -obuild/Print.cpp.o
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/Tone.cpp -obuild/Tone.cpp.o
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/WMath.cpp -obuild/WMath.cpp.o
	avr-g++ $(G++FLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Ilibraries libraries/arduino/WString.cpp -obuild/WString.cpp.o
	avr-ar rcs build/core.a build/pins_arduino.c.o
	avr-ar rcs build/core.a build/WInterrupts.c.o
	avr-ar rcs build/core.a build/wiring.c.o
	avr-ar rcs build/core.a build/wiring_analog.c.o
	avr-ar rcs build/core.a build/wiring_digital.c.o
	avr-ar rcs build/core.a build/wiring_pulse.c.o
	avr-ar rcs build/core.a build/wiring_shift.c.o
	avr-ar rcs build/core.a build/HardwareSerial.cpp.o
	avr-ar rcs build/core.a build/main.cpp.o
	avr-ar rcs build/core.a build/Print.cpp.o
	avr-ar rcs build/core.a build/Tone.cpp.o
	avr-ar rcs build/core.a build/WMath.cpp.o
	avr-ar rcs build/core.a build/WString.cpp.o
	
	avr-g++ $(GCCFLAGS) -mmcu=atmega1280 -DF_CPU=$(CLOCK) -DARDUINO=20 -Iarduino -I libraries telduino.cpp -obuild/telduino.o
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega1280 -o build/telduino.elf build/telduino.o build/core.a -Larduino -lm 
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 build/telduino.elf build/telduino.eep 
	avr-objcopy -O ihex -R .eeprom build/telduino.elf build/telduino.hex 



program: $(PROJECT).hex
	avrdude -patmega1280 -cusbtiny -Uflash:w:build/$(PROJECT).hex

