CC = mpic++

SRC  = chrono.c
MAIN = myBroadcast

CFLAGS = -I./
LDLIBS = -lmpi

all: $(SRC) $(MAIN)

clean:
	@ rm -f $(MAIN)

.PHONY: all clean
