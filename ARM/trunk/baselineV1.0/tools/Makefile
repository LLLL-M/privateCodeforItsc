CFLAGS=-s -unaligned_access -Wall -Wno-array-bounds  -Wno-implicit-function-declaration -O3 #-DDEBUG
CFLAGS+=-I../common/ -I../libs/hikconfig/ -I../libs/parse_ini/ -I../libs/LogSystem/
SLIB=../libs/hikconfig/libhikconfig.a ../libs/libits.a ../libs/libsocketcan.a

COMMON=../common/inifile.o ../common/binfile.o

CC=arm-none-linux-gnueabi-gcc

all: clean BinaryTextConvert
	$(RM) *.o
BinaryTextConvert: BinaryTextConvert.o $(COMMON) $(SLIB)

clean:
	$(RM) *.o BinaryTextConvert
