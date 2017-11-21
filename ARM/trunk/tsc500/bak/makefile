CROSS_COMPILE ?= arm-none-linux-gnueabi-
CFLAGS=-Wall -s -unaligned_access -Wno-implicit-function-declaration -Wno-unused-variable -Wno-array-bounds
CFLAGS+= -O3

CC = $(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

all: clean hikTSC
	$(RM) *.o

hikTSC: ./NGSL_io.o ./CPLD.o ./inifile.o  ./udpserver.o ./countDown.o ./HikConfig.o \
        ./libits.a ./libcanits.a ./libparseini.a
	$(CC) $^ -lpthread  -o $@ 
##	$(STRIP) $@


clean:
	$(RM)  *.o hikTSC
