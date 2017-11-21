CROSS_COMPILE ?= arm-none-linux-gnueabi-
CFLAGS=-s -unaligned_access -Wall -Wno-array-bounds -O3 #-DDEBUG
LDFLAGS=-lpthread

CC = $(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

all: clean hikTSC
	$(RM) *.o

hikTSC: NGSL_io.o io.o inifile.o udpserver.o countDown.o canmsg.o HikConfig.o libparseini.a libsocketcan.a libits.a
	$(CC) $^ -o $@ -lpthread

clean:
	$(RM) *.o hikTSC 
