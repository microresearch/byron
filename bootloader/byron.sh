#!/bin/bash
#
# load a new Byron firmware application
# currently choices are: usbasp programmer, HID, BYRON and keyboard
#

clear
echo "BYRON firmware menu. Choose an app to load:"
echo "1] programmer"
echo "2] HID"
echo "3] BYRON himself"
echo "4] USB keyboard"
echo "5] MIDI"
echo "6] Ready for Arduino (jumper to return)"
echo -n "Please enter option [1 - 6]"
read opt
case $opt in
    1) echo "Loading programmer..."
	../bootloader/byronjump/byronjump
	sleep 2
	avrdude -c usbasp -p m8 -U flash:w:../programmer/firmware/main.hex;;
    2) echo "Loading HID..."
	../bootloader/byronjump/byronjump
	sleep 2
	avrdude -c usbasp -p m8 -U flash:w:../HID/main.hex;;
    3) echo "Loading BYRON..."
	../bootloader/byronjump/byronjump
	sleep 2
	avrdude -c usbasp -p m8 -U flash:w:../byron/main.hex;;
    4) echo "Loading keyboard..."
	../bootloader/byronjump/byronjump
	sleep 2
	avrdude -c usbasp -p m8 -U flash:w:../keyboard/main.hex;;
    5) echo "Loading MIDI..."
	../bootloader/byronjump/byronjump
	sleep 2
	avrdude -c usbasp -p m8 -U flash:w:../MIDI/main.hex;;
    6) echo "Preparing for Arduino..."
	../bootloader/byronjump/byronjump
	sleep 2

esac
