CFLAGS=-s -unaligned_access -Wall -Wno-array-bounds  -Wno-implicit-function-declaration -O3
CFLAGS+=-I../hik/common/ -I../include/ -I../gb/gbconfig/ -I../hik/hikconfig/ -I../libs/parse_ini/ -I../libs/logsystem/ -I../youkong/ykconfig/

COMMON=../hik/common/inifile.o ../hik/common/binfile.o

ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

ifeq ($(ARCH),pc)
CC=gcc
AR=ar
SLIB+=../hik/hikconfig/libpchikconfig.a ../youkong/ykconfig/libpcykconfig.a
else
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
SLIB+=../hik/hikconfig/libhikconfig.a 
endif


all: clean BinaryTextConvert
	$(RM) *.o
BinaryTextConvert: BinaryTextConvert.o $(COMMON) $(SLIB)

clean:
	$(RM) *.o BinaryTextConvert
