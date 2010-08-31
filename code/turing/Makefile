# Name: Makefile
# Project: turing
# Author: Zach Banks
# Creation Date: 2010-08-11
# Tabsize: 4
# Copyright: (c) 2010 olopede llc
# License: GNU GPL v2

DEVICE  = atmega168
F_CPU   = 8000000	# in Hz
#FUSE_L  = 
#FUSE_H  = 
AVRDUDE = avrdude -c dapa -P /dev/parport0 -p $(DEVICE) # edit this line for your programmer
#AVRDUDE = avrdude -p $(DEVICE) -P /dev/ttyUSB0 -c avrisp -b 19200 -F
#AVRDUDE = avrdude -p $(DEVICE) -c usbasp

# Excellent resource for optimizing cflags: http://www.tty1.net/blog/2008-04-29-avr-gcc-optimisations_en.html
CPPFLAGS  = -I. -DDEBUG_LEVEL=0 -Os -W -Wall -pedantic -Wundef -Werror -funsigned-char -funsigned-bitfields -ffunction-sections -fpack-struct -fshort-enums -ffreestanding -g -gdwarf-2  -fno-inline-small-functions -fno-split-wide-types -fno-tree-scev-cprop -Wl,--relax,--gc-sections 
CFLAGS = $(CPPFLAGS) -std=gnu99  -Wstrict-prototypes
OBJECTS = interface.o turing.o main.o
COBJECTS = interface.cpp turing.cpp main.cpp

COMPILE = avr-gcc -Wall  -DF_CPU=$(F_CPU) -mmcu=$(DEVICE)


# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo "make hex ....... to build main.hex"
	@echo "make program ... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make clean ..... to delete objects and hex file"

hex: main.hex

program: flash fuse

# rule for programming fuse bits:
fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

# rule for uploading firmware:
flash: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.elf *.o 

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(COMPILE) $(CPPFLAGS) -c $< -o $@ 

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

# file targets:

# Use -fwhole-program for huge, free space reductions
main.elf: $(OBJECTS)
	$(COMPILE) $(CPPFLAGS) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size main.hex


# debugging targets:

disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
