ifeq "$(origin ARCH)" "undefined"
$(warning You can run 'make ARCH=pc or ARCH=arm')
ARCH=arm
endif

OPTIMIZER = -O3
CFLAGS = $(OPTIMIZER) -Wall -Werror
LDFLAGS = -lpthread -lrt -ldl

PGM = TrafficSignalController
PGMOBJS = Main.o CPLD.o inifile.o countDown.o udpserver.o libits.a

LIBOBJS = ItsMain.o Util.o LogSystem.o light.o strategy.o Database.o

ifeq ($(ARCH),pc)
CC=gcc
AR=ar
HIKCONFIGLIB=libpchikconfig.a
LDFLAGS += -lsqlite3
LIBOBJS+=$(shell $(AR) -t $(HIKCONFIGLIB))
else
CC = arm-none-linux-gnueabi-gcc
AR = arm-none-linux-gnueabi-ar
HIKCONFIGLIB=libhikconfig.a
CFLAGS += -DARM_PLATFORM
PGMOBJS += libcanits.a
LIBOBJS+=$(shell $(AR) -t libsqlite3.a)
LIBOBJS+=$(shell $(AR) -t $(HIKCONFIGLIB))
endif



all: clean libits.a $(PGM)
	$(RM) *.o

$(PGM): $(PGMOBJS) 
	$(CC) -o $(PGM) $^ $(LDFLAGS)

libits.a: xlib $(LIBOBJS)
	$(AR) crs $@ $(LIBOBJS)

xlib: libsqlite3.a libhikconfig.a
	$(AR) -x libsqlite3.a
	$(AR) -x $(HIKCONFIGLIB)

clean:
	$(RM) *.o $(PGM) libits.a



