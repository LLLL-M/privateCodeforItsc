CFLAGS=-s -unaligned_access -Wall -Wno-array-bounds  -Wno-implicit-function-declaration -O3
CFLAGS+=-I../common/ -I../include/ -I../libs/gbconfig/ -I../libs/hikconfig/ -I../libs/parse_ini/ -I../libs/LogSystem/ -I../libs/ykconfig/

COMMON=../common/inifile.o ../common/binfile.o

ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

ifeq ($(ARCH),pc)
CC=gcc
AR=ar
SLIB+=../libs/gbconfig/libpcgbconfig.a ../libs/hikconfig/libpchikconfig.a ../libs/ykconfig/libpcykconfig.a
else
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
SLIB+=../libs/gbconfig/libgbconfig.a ../libs/hikconfig/libhikconfig.a ../libs/ykconfig/libykconfig.a
endif


all: clean BinaryTextConvert
	$(RM) *.o
BinaryTextConvert: BinaryTextConvert.o $(COMMON) $(SLIB)

clean:
	$(RM) *.o BinaryTextConvert
