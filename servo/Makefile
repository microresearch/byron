# Name: Makefile
# Project: Byron
# Author: Martin Howse m@1010.co.uk
# Based on Makefile for Powerswitch
# Author: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GPL2
# Target MCU: Atmel Atmega8

DEVICE = byron

#COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=atmega8

COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=atmega8 -Wl,--section-start,.BOOTSTART=0x800060 -Wl,--section-start,.data=0x800064  -Wl,--section-start,.BOOTFLASHSTART=0x17FF

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o byron.o main.o
# Note that we link usbdrv.o first! This is required for correct alignment of
# driver-internal global variables!

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.bin *.o usbdrv/*.o byron.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
main.bin:	$(OBJECTS)
	$(COMPILE) -o main.bin $(OBJECTS)

main.hex:	main.bin
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex


disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	$(COMPILE) -E main.c
