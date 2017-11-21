CROSS_COMPILE ?= arm-none-linux-gnueabi-
CFLAGS=-Wall -s -unaligned_access -Wno-implicit-function-declaration -Wno-array-bounds
CFLAGS+= -O3

CC = $(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

all: clean hikTSC BinaryTextConvert
	$(RM) *.o

hikTSC: ./NGSL_io.o ./CPLD.o ./platform.o ./inifile.o  ./udpserver.o ./countDown.o ./HikConfig.o \
        ./libits.a ./libcanits.a ./libhikconfig.a ./stepControl.o ./specialControl.o 
	$(CC) $^ -lpthread  -o $@ 
	
BinaryTextConvert: 
		$(CC) -o $@ BinaryTextConvert.c ./libhikconfig.a	
	
##	$(STRIP) $@


clean:
	$(RM)  *.o hikTSC BinaryTextConvert
