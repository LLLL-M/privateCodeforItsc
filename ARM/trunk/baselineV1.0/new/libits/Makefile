ifeq "$(origin ARCH)" "undefined"
$(warning You can run 'make ARCH=pc or ARCH=arm')
ARCH=arm
endif

OPTIMIZER = -g -DDEBUG
CFLAGS = $(OPTIMIZER) -Wall -unaligned_access -Wno-unused-variable -Wno-array-bounds -Wno-strict-aliasing  
#	-Wno-implicit-function-declaration -Wno-unused-function
CFLAGS+= -I./ -I../include -I../libs/hikconfig/ -I../libs/logsystem/ -Wl,--hash-style=sysv
LDFLAGS = -lpthread -lrt -ldl

SRC:=$(wildcard *.c)
LIBOBJS:=$(SRC:.c=.o)
 
#使用国标协议需要添加这两行，如果不用就注释掉
#LIBOBJS+= gbinit.o gbconvert.o gbNetServer.o gbNetDecode.o ntcipconvert.o
#CFLAGS+= -DUSE_GB_PROTOCOL

ifeq ($(ARCH),pc)
CC=gcc
AR=ar
SLIB=libpcits.a
DLIB=libpcits.so
LOGSYSTEMLIB=../libs/logsystem/libpclogsystem.a
LIBOBJS+=$(shell $(AR) -t $(LOGSYSTEMLIB))
CFLAGS += -DFOR_SERVER
else
CC = arm-none-linux-gnueabi-gcc
AR = arm-none-linux-gnueabi-ar
SLIB=libits.a
DLIB=libits.so
LOGSYSTEMLIB=../libs/logsystem/liblogsystem.a
LIBOBJS+=$(shell $(AR) -t $(LOGSYSTEMLIB))
endif



all: clean xlib $(SLIB) $(DLIB)
	$(RM) *.o

$(SLIB): $(LIBOBJS)
	$(AR) crs $@ $(LIBOBJS)

$(DLIB): $(LIBOBJS)
	$(CC) -shared $(LDFLAGS) $^ -o $@ #-Wl,-soname,$(LIBNAME).so.$(VER)

xlib: $(LOGSYSTEMLIB)
	$(AR) -x $(LOGSYSTEMLIB)

clean:
	$(RM) *.o libits.a libits.so

