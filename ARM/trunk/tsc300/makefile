CROSS_COMPILE ?= arm-none-linux-gnueabi-
CFLAGS=-s -unaligned_access -Wall -Wno-array-bounds  -Wno-implicit-function-declaration -O3 #-DDEBUG
LDFLAGS=-lpthread

CC = $(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

all: clean hikTSC BinaryTextConvert
	$(RM) *.o

hikTSC: NGSL_io.o io.o inifile.o udpserver.o countDown.o canmsg.o HikConfig.o platform.o  stepControl.o libparseini.a \
		libsocketcan.a libits.a ./libhikconfig.a specialControl.o countDown_LaiSi.o
	$(CC) $^ -o $@ -lpthread

BinaryTextConvert: 
		$(CC) -o $@ BinaryTextConvert.c ./libhikconfig.a
clean:
	$(RM) *.o hikTSC BinaryTextConvert
