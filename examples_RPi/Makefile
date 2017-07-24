#############################################################################
#
# Makefile for librf24 examples on Raspberry Pi
#
# License: GPL (General Public License)
# Author:  gnulnulf <arco@appeltaart.mine.nu>
# Date:    2013/02/07 (version 1.0)
#
# Description:
# ------------
# use make all and make install to install the examples
# You can change the install directory by editing the prefix line
#


REL_RF24_MAKEFILE_INC="../../RF24/Makefile.inc"
RF24_MAKEFILE_INC=$(shell echo "$$(cd "$$(dirname "$(REL_RF24_MAKEFILE_INC)")"; pwd)/$$(basename "$(REL_RF24_MAKEFILE_INC)")")

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
# CFLAGS #
## Assuming Raspberry Pi (original) / Raspberry Pi Zero ##
CFLAGS=-march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -O2 -pthread -pipe -fstack-protector --param=ssp-buffer-size=4 -std=c++0x
## -- Check for Raspberry Pi 2+ -- ##
ifeq "$(shell uname -m)" "armv7l"
## Set CFLAGS for Raspberry Pi 2+ ##
CFLAGS=-march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O2 -pthread -pipe -fstack-protector --param=ssp-buffer-size=4 -std=c++0x
endif

endif
# (end of RF24_MAKEFILE_INC_EXISTS)


# define all programs
PROGRAMS = rx-test helloworld_rx helloworld_tx
SOURCES = ${PROGRAMS:=.cpp}

all: ${PROGRAMS}

${PROGRAMS}: ${SOURCES}
	${CXX} ${CFLAGS} -Wall -I../ $@.cpp -lrf24-bcm -lrf24network -o $@

clean:
	rm -rf $(PROGRAMS)

install:
	test -d $(PREFIX) || mkdir $(PREFIX)
	test -d $(EXAMPLES_DIR) || mkdir $(EXAMPLES_DIR)
	for prog in $(PROGRAMS); do \
	  install -m 0755 $$prog $(EXAMPLES_DIR); \
	done

# simple debug function
print-%:
	@echo $*=$($*)

.PHONY: install
