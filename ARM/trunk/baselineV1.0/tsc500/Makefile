CROSS_COMPILE ?= arm-none-linux-gnueabi-
CFLAGS=-Wall -s -unaligned_access -Wno-implicit-function-declaration #-Wno-array-bounds
CFLAGS+= -O3 -Wl,--hash-style=sysv
LDFLAGS= -lpthread -lrt
#信号机软件程序版本硬件版本,长度控制在32个字符以内，否则sadp获取会异常
DATE=$(shell date +%Y.%m.%d)
SOFTWARE_VERSION="HIKTSC-V1.1.0.0-$(DATE)"
HARDWARE_VERSION="DS-TSC500"
CFLAGS+=-DSOFTWARE_VERSION_INFO=\"$(SOFTWARE_VERSION)\"
CFLAGS+=-DHARDWARE_VERSION_INFO=\"$(HARDWARE_VERSION)\"
CFLAGS+=-I../common/ -I../libs/hikconfig/ -I../libs/parse_ini/ -I../libs/LogSystem/
SLIB=../libs/hikconfig/libhikconfig.a ../libs/libits.a ../libs/libsocketcan.a ../libs/LogSystem/liblogsystem.a

COMMON:=$(wildcard ../common/*.c)
COMMON:=$(COMMON:.c=.o)

CC = $(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

all: clean hikTSC 
	$(RM) *.o ../common/*.o

hikTSC: NGSL_io.o CPLD.o canmsg.o $(COMMON) $(SLIB)
	$(CC) $^ $(LDFLAGS)  -o $@ -lm 
	
##	$(STRIP) $@


clean:
	$(RM) *.o ../common/*.o hikTSC 
