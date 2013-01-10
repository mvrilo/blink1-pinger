# Mac OS X
#   - Install Xcode and command-line dev tools
#   - Install Homebrew
#   - brew install libusb-compat
#
# Linux (Ubuntu) 
#   - apt-get install gcc-avr avr-libc avrdude java librxtx-java
#
# OpenWrt / DD-WRT
#   - Download the OpenWrt SDK for Linux (only for Linux now, I think)
#   - set WRT_SDK_HOME environment variable
#   - type "make OS=wrt" to build
#
# BeagleBone / BeagleBoard (on Angstrom Linux)
#   - Install USB dev support 
#      "opkg install libusb-0.1-4-dev"
#   - May need to symlink libusb 
#      "cd /lib; ln -s libusb-0.1.so.4 libusb.so"

CC = gcc
PROGRAM  = blink1-pinger
USBFLAGS = `libusb-config --cflags`
USBLIBS  = `libusb-config --libs`
ifeq "$(OS)" ""
OS = `uname`
endif

# OpenWrt / DD-WRT
ifeq "$(OS)" "wrt"
WRT_SDK_HOME := $(HOME)/OpenWrt-SDK-Linux-i686-1
CC = $(WRT_SDK_HOME)/staging_dir_mipsel/bin/mipsel-linux-gcc
LD = $(WRT_SDK_HOME)/staging_dir_mipsel/bin/mipsel-linux-ld
USBFLAGS = "-I$(WRT_SDK_HOME)/staging_dir_mipsel/usr/include"
USBLIBS  = "$(WRT_SDK_HOME)/staging_dir_mipsel/usr/lib/libusb.a"
endif

CFLAGS = $(OS_CFLAGS) -O -Wall -std=gnu99 $(USBFLAGS) -Ideps
LIBS = $(OS_LIBS) $(USBLIBS) -Ideps -lev

SRC = $(PROGRAM).c deps/hiddata.c deps/blink1-lib.c
OBJ = $(SRC:.c=.o)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

otool:
	otool -L $(PROGRAM)
