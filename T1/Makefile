CC = g++
VERSION = v1

SRC  = chrono.c
MAIN = prefixSumPth-$(VERSION)

CFLAGS  ?= -O3
DBGFLAGS = -g
LDLIBS   = -lpthread

all: $(SRC) $(MAIN)

debug:
	@ CFLAGS=$(DBGFLAGS) $(MAKE)

clean:
	@ rm -f $(MAIN)

.PHONY: clean
 
#	g++ -mcmodel=large prefixSumPth.c -O3 -o prefixSumPth -lpthread