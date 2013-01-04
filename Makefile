# Makefile for "lib/blink1-lib" and "blink1-tool"
# should work on Mac OS X, Windows, Linux, and other Linux-like systems
#  
# Build arguments:
# - "OS=macosx"  -- build Mac version on Mac OS X
# - "OS=windows" -- build Windows version on Windows
# - "OS=linux"   -- build Linux version on Linux
# 
# Architecture is usually detected automatically, so normally just type "make"
#
# Dependencies: 
# - hidapi (included), which uses libusb on Linux-like OSes
#
# Platform-specific notes:
#
# Mac OS X 
#   - Install Xcode with "Unix Dev Support" and "Command-line tools"
#   - make
#
# Windows XP/7  
#   - Install MinGW and MSYS (http://www.tdragon.net/recentgcc/ )
#   - make
#
# Linux (Ubuntu) 
#   - apt-get install gcc-avr avr-libc   (to build firmware files)
#   - apt-get install libusb-1.0-0-dev
#   - make
#
# FreeBSD
#   - libusb is part of the OS so no pkg-config needed.
#   - No -ldl on FreeBSD necessary.
#   - iconv is a package that needs to be installed; it lives in /usr/local/lib/
#
# Linux Ubuntu 32-bit cross-compile on 64-bit
#   To build 32-bit on 64-bit Ubuntu, try a chrooted build:
#   (warning this will use up a lot of disk space)
#   - sudo apt-get install ubuntu-dev-tools
#   - pbuilder-dist oneiric i386 create
#   - mkdir $HOME/i386
#   - cp -r blink1 $HOME/i386
#   - pbuilder-dist oneiric i386 login --bindmounts $HOME/i386
#     (now in the chrooted area)
#   - apt-get install libusb-1.0-0 libusb-1.0-0-dev
#   - cd $HOME/i386/blink1
#   - CFLAGS='-I/usr/include/libusb-1.0' LIBS='-lusb-1.0' make
#   - exit
#   
# Raspberry Pi
#   - apt-get install libusb-1.0.0-dev
#   - make
#
# BeagleBone / BeagleBoard (on Angstrom Linux)
#   - opkg install libusb-0.1-4-dev  (FIXME: uses HIDAPI & libusb-1.0 now)	
#   - May need to symlink libusb 
#      cd /lib; ln -s libusb-0.1.so.4 libusb.so
#   - make
#
#

# try to do some autodetecting
UNAME := $(shell uname -s)

ifeq "$(UNAME)" "Darwin"
	OS=macosx
endif

ifeq "$(OS)" "Windows_NT"
	OS=windows
endif

ifeq "$(UNAME)" "Linux"
	OS=linux
endif

ifeq "$(UNAME)" "FreeBSD"
	OS=freebsd
endif

ifeq "$(PKGOS)" ""
   PKGOS = $(OS)
endif


CC=gcc

#################  Mac OS X  ##################################################
ifeq "$(OS)" "macosx"
LIBTARGET = libBlink1.dylib
CFLAGS += -arch x86_64 #-arch i386
CFLAGS += -pthread
LIBS += -framework IOKit -framework CoreFoundation
OBJS = ./hidapi/mac/hid.o

EXEFLAGS = 
LIBFLAGS = -bundle -o $(LIBTARGET) -Wl,-search_paths_first $(LIBS)

EXE=
endif

#################  Windows  ##################################################
ifeq "$(OS)" "windows"
LIBTARGET = lib/blink1-lib.dll
LIBS += -lsetupapi -Wl,--enable-auto-import -static-libgcc -static-libstdc++ 
OBJS = ./hidapi/windows/hid.o

EXEFLAGS =
LIBFLAGS = -shared -o $(LIBTARGET) -Wl,--add-stdcall-alias -Wl,--export-all-symbols

EXE= .exe
endif

#################  Linux  ####################################################
ifeq "$(OS)" "linux"
LIBTARGET = lib/blink1-lib.so
CFLAGS += `pkg-config libusb-1.0 --cflags` -fPIC
LIBS   += `pkg-config libusb-1.0 --libs` -lrt -lpthread -ldl

OBJS = ./hidapi/libusb/hid.o

EXEFLAGS = -static
LIBFLAGS = -shared -o $(LIBTARGET) $(LIBS)

EXE=
endif

################  FreeBSD  ###################################################
ifeq "$(OS)" "freebsd"
LIBTARGET = lib/blink1-lib.so
LIBS   += -L/usr/local/lib -lusb -lrt -lpthread -liconv -static
OBJS = ./hidapi/libusb/hid.o
EXEFLAGS=
LIBFLAGS = -shared -o $(LIBTARGET) $(LIBS)
EXE=
endif

#####################  Common  ###############################################

CFLAGS += -std=gnu99 -lev
CFLAGS += -I./firmware -I./hidapi/hidapi -I./lib

OBJS += lib/blink1-lib.o

all: msg blink1-pinger

msg: 
	@echo "building for OS=$(OS)"

# symbolic targets:
help:
	@echo "This Makefile works on multiple archs. Use one of the following:"
	@echo "make OS=windows ... build Windows  lib/blink1-lib and blink1-tool" 
	@echo "make OS=linux   ... build Linux    lib/blink1-lib and blink1-tool" 
	@echo "make OS=freebsd ... build FreeBSD    lib/blink1-lib and blink1-tool" 
	@echo "make OS=macosx  ... build Mac OS X lib/blink1-lib and blink1-tool" 
	@echo "make lib        ... build lib/blink1-lib shared library"
	@echo "make package PKGOS=mac  ... zip up build, give it a name 'mac' "
	@echo "make clean ..... to delete objects and hex file"
	@echo

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

blink1-pinger: $(OBJS) blink1-pinger.o
	$(CC) $(CFLAGS) -c blink1-pinger.c -o blink1-pinger.o
	$(CC) $(CFLAGS) $(EXEFLAGS) -g $(OBJS) $(LIBS) blink1-pinger.o -o blink1-pinger$(EXE) 

lib: $(OBJS)
	$(CC) $(LIBFLAGS) $(CFLAGS) $(OBJS) $(LIBS)

clean: 
	rm -f $(OBJS) 
	rm -f blink1-pinger.o

distclean: clean
	rm -f blink1-pinger$(EXE)
	rm -f $(LIBTARGET) $(LIBTARGET).a

# shows shared lib usage on Mac OS X
otool:
	otool -L blink1-pinger
# show shared lib usage on Linux
ldd:
	ldd blink1-pinger
# show shared lib usage on Windows
# FIXME: only works inside command prompt from
# Start->All Programs-> MS Visual Studio 2012 -> VS Tools -> Devel. Cmd Prompt
dumpbin: 
	dumpbin.exe /exports $(LIBTARGET)
	dumpbin.exe /exports blink1-pinger.exe

foo:
	@echo "OS=$(OS), USBFLAGS=$(USBFLAGS)"

