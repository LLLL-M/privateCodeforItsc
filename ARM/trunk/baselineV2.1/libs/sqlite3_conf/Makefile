ifeq "$(origin ARCH)" "undefined"
$(warning You can run 'make ARCH=pc or ARCH=arm')
ARCH=arm
endif

ifeq "$(origin USELIB)" "undefined"
$(warning "You can run make USELIB=s or USELIB=d, which decide to use static lib or dynamic lib")
USELIB=d
endif

OPTIMIZER = -g -DDEBUG
CFLAGS = $(OPTIMIZER) -Wall  -unaligned_access -Wno-unused-variable -Wno-array-bounds -Wno-strict-aliasing 
#-Wno-implicit-function-declaration 
CFLAGS+= -I./ -I../../include -I../../hik/hikconfig -I../../libs/logsystem/ -I../../hik/common   -Wl,--hash-style=sysv
LDFLAGS = -lpthread -lrt -ldl
LIBSQLITE= 

ifeq ($(ARCH),pc)
	ifeq ($(USELIB),d)
	LDFLAGS+= -lsqlite3
	else
	LIBSQLITE = ../libpcsqlite3.a
	endif
else
	ifeq ($(USELIB),d)
	LDFLAGS+=  -L../ -lsqlite3
	else
	LIBSQLITE = ../libsqlite3.a
	endif
endif

SRC:=$(wildcard *.c)
LIBOBJS:=$(SRC:.c=.o)


ifeq ($(ARCH),pc)
CC=gcc -m32
AR=ar
LIBNAME=libpcsqliteconf
APP=sqliteConvert_pc
else
CC = arm-none-linux-gnueabi-gcc
AR = arm-none-linux-gnueabi-ar
LIBNAME=libsqliteconf
APP=sqliteConvert
endif

SLIB=$(LIBNAME).a
DLIB=$(LIBNAME).so

all: clean $(SLIB) $(DLIB) $(APP)
	$(RM) *.o
	
#sqliteConvert: $(LIBOBJS)
#	$(CC) -o $@ $^  $(CFLAGS)  $(LDFLAGS)

$(APP): sqlite_convert.o   $(SLIB) $(LIBSQLITE)
		$(CC) $^  -o $@  $(CFLAGS)  $(LDFLAGS)  
sqlite_convert.o:

$(SLIB): sqlite_conf.o 
	$(AR) crs $@ $^ 

$(DLIB): sqlite_conf.o
	$(CC) -shared -fPIC $^  -o $@ #-Wl,-soname,$(LIBNAME).so
sqlite_conf.o:

clean:
	$(RM) *.o $(APP) $(SLIB) $(DLIB)
