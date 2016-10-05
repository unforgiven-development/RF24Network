#############################################################################
#
# Makefile for librf24network-bcm on Raspberry Pi
#
# License: GPL (General Public License)
# Author:  Charles-Henri Hallard 
# Date:    2013/03/13 
#
# Description:
# ------------
# use make all and mak install to install the library 
# You can change the install directory by editing the LIBDIR line
#

DEBUG_MAKEFILE=1

# Check to see if ../RF24/Makefile.inc exists (ie: all of the RF24xxx projects were cloned in a common folder)
RF24_MAKEFILE_INC="../RF24/Makefile.inc"

ifneq ("$(wildcard $(RF24_MAKEFILE_INC))","")
RF24_MAKEFILE_INC_EXISTS=1
else
RF24_MAKEFILE_INC_EXISTS=0
endif

ifeq ($(RF24_MAKEFILE_INC_EXISTS),1)
# $(RF24_MAKEFILE_INC) DEFINES:
# - CFLAGS
# - PREFIX
# - CC
# - CXX
# - LDCONFIG
# - LIB_DIR
# - EXAMPLES_DIR
include $(RF24_MAKEFILE_INC)
else
PREFIX=/usr/local
LIB_DIR=$(PREFIX)/lib
EXAMPLES_DIR=$(PREFIX)/bin
CC=gcc
CXX=g++
LDCONFIG=ldconfig

# Assuming Raspberry Pi (original) / Raspberry Pi Zero
CFLAGS=-O2 -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -std=c++0x
# -- Check for Raspberry Pi 2+ --
ifeq "$(shell uname -m)" "armv7l"
# Set $CFLAGS for Raspberry Pi 2+
CFLAGS=-march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O2 -pthread -pipe -fstack-protector --param=ssp-buffer-size=4 -std=c++0x
endif

endif
# (end of RF24_MAKEFILE_INC_EXISTS)

# lib name
LIB_RFN=librf24network
# shared library name
LIBNAME_RFN=$(LIB_RFN).so.1.0
# header directory
HEADER_DIR=${PREFIX}/include/RF24Network

# make all
# reinstall the library after each recompilation
all: librf24network

# Make the library
librf24network: RF24Network.o
	${CXX} -shared -Wl,-soname,$@.so.1 ${CFLAGS} -o ${LIBNAME_RFN} $^ -lrf24-bcm

# Library parts
RF24Network.o: RF24Network.cpp
	${CXX} -Wall -fPIC ${CFLAGS} -c $^

# clear build files
clean:
	rm -rf *.o ${LIB_RFN}.*

install: install-libs install-headers

# Install the library to LIB_DIR

install-libs: 
	@echo "[Install]"
	@if ( test ! -d $(PREFIX)/lib ) ; then mkdir -p $(PREFIX)/lib ; fi
	@install -m 0755 ${LIBNAME_RFN} ${LIB_DIR}
	@ln -sf ${LIB_DIR}/${LIBNAME_RFN} ${LIB_DIR}/${LIB_RFN}.so.1
	@ln -sf ${LIB_DIR}/${LIBNAME_RFN} ${LIB_DIR}/${LIB_RFN}.so
	@ldconfig

install-headers:
	@echo "[Installing Headers]"
	@if ( test ! -d ${HEADER_DIR} ) ; then mkdir -p ${HEADER_DIR} ; fi
	@install -m 0644 *.h ${HEADER_DIR}

# simple debug function
print-%:
	@echo $*=$($*)
