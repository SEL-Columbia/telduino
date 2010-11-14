# this is all library object code stuff
# -c assemble but do not link
# -g include debug stuff
# -Os optimize for size
# -w inhibit all warning messages
# -ffunction-sections and -fdata-sections are for optimization of code location
# -I adds directory to the head of the list to be searched for header files
# -o place the output in file

# atmel 1280 comes with a default clock prescaling of 8, but that is fixed in the telduino setup
TARGETARCH  = atmega1280
CLOCK = 8000000L
PROJECT = telduino
GCCFLAGS = -c -g -Os -w  -ffunction-sections -fdata-sections -Ilibraries -mmcu=$(TARGETARCH) -DF_CPU=$(CLOCK)
G++FLAGS = -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -Ilibraries -mmcu=$(TARGETARCH) -DF_CPU=$(CLOCK)

arduino: telduino.cpp
	#First rule
	avr-gcc $(GCCFLAGS) libraries/arduino/pins_arduino.c 	-obuild/pins_arduino.c.o
	avr-gcc $(GCCFLAGS) libraries/arduino/WInterrupts.c 	-obuild/WInterrupts.c.o
	avr-gcc $(GCCFLAGS) libraries/arduino/wiring.c 		-obuild/wiring.c.o
	avr-gcc $(GCCFLAGS) libraries/arduino/wiring_analog.c 	-obuild/wiring_analog.c.o
	avr-gcc $(GCCFLAGS) libraries/arduino/wiring_digital.c 	-obuild/wiring_digital.c.o
	avr-gcc $(GCCFLAGS) libraries/arduino/wiring_pulse.c 	-obuild/wiring_pulse.c.o
	avr-gcc $(GCCFLAGS) libraries/arduino/wiring_shift.c 	-obuild/wiring_shift.c.o 
	
	#SR, Demux
	avr-gcc $(GCCFLAGS) libraries/Demux/Demux.c 		-obuild/Demux.c.o 
	avr-gcc $(GCCFLAGS) libraries/ShiftRegister/ShiftRegister.c -obuild/ShiftRegister.c.o 
	
	avr-g++ $(G++FLAGS) libraries/arduino/HardwareSerial.cpp -obuild/HardwareSerial.cpp.o
	avr-g++ $(G++FLAGS) libraries/arduino/main.cpp 		-obuild/main.cpp.o
	avr-g++ $(G++FLAGS) libraries/arduino/Print.cpp 	-obuild/Print.cpp.o
	#avr-g++ $(G++FLAGS) libraries/arduino/Tone.cpp -obuild/Tone.cpp.o
	avr-g++ $(G++FLAGS) libraries/arduino/WMath.cpp 	-obuild/WMath.cpp.o
	avr-g++ $(G++FLAGS) libraries/arduino/WString.cpp 	-obuild/WString.cpp.o
	avr-g++ $(G++FLAGS) libraries/arduino/WString.cpp 	-obuild/WString.cpp.o
	
	#SPI, ADE, SD
	avr-g++ $(G++FLAGS) libraries/SPI/SPI.cpp 		-obuild/SPI.cpp.o
	avr-g++ $(G++FLAGS) libraries/ADE7753/ADE7753.cpp 	-obuild/ADE7753.cpp.o
	avr-g++ $(G++FLAGS) libraries/SDRaw/sd_raw.cpp 		-obuild/sd_raw.cpp.o
	avr-g++ $(G++FLAGS) libraries/DbgTel/DbgTel.cpp 	-obuild/DbgTel.cpp.o
	avr-g++ $(G++FLAGS) libraries/Select/select.cpp 	-obuild/select.cpp.o
	
	#Second rule
	avr-ar rcs build/core.a build/pins_arduino.c.o
	avr-ar rcs build/core.a build/WInterrupts.c.o
	avr-ar rcs build/core.a build/wiring.c.o
	avr-ar rcs build/core.a build/wiring_analog.c.o
	avr-ar rcs build/core.a build/wiring_digital.c.o
	avr-ar rcs build/core.a build/wiring_pulse.c.o
	avr-ar rcs build/core.a build/wiring_shift.c.o
	#SR,Demux
	avr-ar rcs build/core.a build/ShiftRegister.c.o
	avr-ar rcs build/core.a build/Demux.c.o
	avr-ar rcs build/core.a build/select.cpp.o
	
	
	avr-ar rcs build/core.a build/HardwareSerial.cpp.o
	avr-ar rcs build/core.a build/main.cpp.o
	avr-ar rcs build/core.a build/Print.cpp.o
	#avr-ar rcs build/core.a build/Tone.cpp.o
	avr-ar rcs build/core.a build/WMath.cpp.o
	avr-ar rcs build/core.a build/WString.cpp.o
	#SPI,ADE
	avr-ar rcs build/core.a build/SPI.cpp.o
	avr-ar rcs build/core.a build/ADE7753.cpp.o
	avr-ar rcs build/core.a build/sd_raw.cpp.o
	avr-ar rcs build/core.a build/DbgTel.cpp.o
	
	
	#first rule
	avr-g++ $(G++FLAGS) telduino.cpp -obuild/telduino.o
	#work
	avr-gcc -Os -Wl,--gc-sections -mmcu=$(TARGETARCH) -o build/telduino.elf build/telduino.o build/core.a -Larduino -lm 
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 build/telduino.elf build/telduino.eep 
	avr-objcopy -O ihex -R .eeprom build/telduino.elf build/telduino.hex 
	avrdude -p$(TARGETARCH) -cdragon_isp -Pusb -Uflash:w:build/$(PROJECT).hex



program: $(PROJECT).hex
	#avrdude -p$(TARGETARCH) -cusbtiny -Uflash:w:build/$(PROJECT).hex
	avrdude -p$(TARGETARCH) -cusbtiny -Uflash:w:build/$(PROJECT).hex
clean: 
	rm build/*
