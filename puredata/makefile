pd_linux: byron.pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O6 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch `libusb-config --libs`

LINUXINCLUDE =  -I/usr/local/lib/pd/include

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c -g
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm -lusb
	strip --strip-unneeded $*.pd_linux
	rm $*.o
