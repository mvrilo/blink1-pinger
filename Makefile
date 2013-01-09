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

TARGET = blink1-pinger

CC = gcc
ifeq "$(OS)" ""
	OS = `uname`
endif

USBFLAGS = `libusb-config --cflags`
USBLIBS  = `libusb-config --libs`

#################  OpenWrt / DD-WRT #########################################
ifeq "$(OS)" "wrt"

WRT_SDK_HOME := $(HOME)/OpenWrt-SDK-Linux-i686-1

CC = $(WRT_SDK_HOME)/staging_dir_mipsel/bin/mipsel-linux-gcc
LD = $(WRT_SDK_HOME)/staging_dir_mipsel/bin/mipsel-linux-ld
USBFLAGS = "-I$(WRT_SDK_HOME)/staging_dir_mipsel/usr/include"
USBLIBS  = "$(WRT_SDK_HOME)/staging_dir_mipsel/usr/lib/libusb.a"

endif

CFLAGS =	$(OS_CFLAGS) -O -Wall -std=gnu99 $(USBFLAGS)
LIBS =	$(OS_LIBS) $(USBLIBS) -lev

OBJ = $(TARGET).o blink1-lib.o hiddata.o

PROGRAM = $(TARGET)$(EXE_SUFFIX)

all: msg $(PROGRAM)

msg: 
	@echo "building for OS=$(OS)"

# symbolic targets:
help:
	@echo "This Makefile works on multiple archs. Use one of the following:"
	@echo "make clean ..... to delete objects and hex file"
	@echo

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)


strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o

# shows shared lib usage on Mac OS X
otool:
	otool -L $(TARGET)

foo:
	@echo "OS=$(OS), USBFLAGS=$(USBFLAGS)"
