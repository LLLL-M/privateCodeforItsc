ifeq "$(origin ARCH)" "undefined"
$(warning You can run 'make ARCH=pc or ARCH=arm')
ARCH=arm
endif

OPTIMIZER = -DDEBUG -fPIC
CFLAGS = $(OPTIMIZER) -Wall -Werror -unaligned_access -Wno-unused-variable -Wno-array-bounds -Wno-strict-aliasing 
#-Wno-implicit-function-declaration 
CFLAGS+= -I./ -I../../include  -I../../hik/common/ -Wl,--hash-style=sysv
LDFLAGS = -lpthread -lrt -ldl

SRC:=$(wildcard *.c)
LIBOBJS:=$(SRC:.c=.o)
 
#使用国标协议需要添加这两行，如果不用就注释掉
#LIBOBJS+= gbinit.o gbconvert.o gbNetServer.o gbNetDecode.o ntcipconvert.o
#CFLAGS+= -DUSE_GB_PROTOCOL

ifeq ($(ARCH),pc)
LIBNAME=libpchikxml
CC=gcc -m32
AR=ar
MXMLLIB=./libpcmxml.a
LIBOBJS+=$(shell $(AR) -t $(MXMLLIB))
else
LIBNAME=libhikxml
CC = arm-none-linux-gnueabi-gcc
AR = arm-none-linux-gnueabi-ar
MXMLLIB=./libmxml.a
LIBOBJS+=$(shell $(AR) -t $(MXMLLIB))
endif

SLIB=$(LIBNAME).a
DLIB=$(LIBNAME).so

all: clean xlib $(SLIB) $(DLIB)
	$(RM) *.o

$(SLIB): $(LIBOBJS)
	$(AR) crs $@ $(LIBOBJS)

$(DLIB): $(LIBOBJS)
	$(CC) -shared $(LDFLAGS) $^ -o $@ #-Wl,-soname,$(LIBNAME).so.$(VER)

xlib: $(MXMLLIB)
	$(AR) -x $(MXMLLIB)

clean:
	$(RM) *.o $(SLIB) $(DLIB)

