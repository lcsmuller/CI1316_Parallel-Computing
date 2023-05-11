VERSION = v1

OBJS = chrono.o
MAIN = prefixSumPth-$(VERSION)

CXXFLAGS ?= -O3
DBGFLAGS = -g
LDLIBS   = -lpthread

all: $(MAIN)

debug:
	@ CXXFLAGS=$(DBGFLAGS) CFLAGS=$(DBGFLAGS) $(MAKE)

$(MAIN): $(OBJS)

clean:
	@ rm -f $(OBJS) $(MAIN)

.PHONY: clean
 
#	g++ -mcmodel=large prefixSumPth.c -O3 -o prefixSumPth -lpthread