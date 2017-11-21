ifeq "$(origin ARCH)" "undefined"
$(warning You can run 'make ARCH=pc or ARCH=arm')
ARCH=pc
endif

OPTIMIZER=-fPIC
#CPPFLAGS=$(OPTIMIZER) -Wall -Werror -unaligned_access -Wno-unused-variable -Wno-array-bounds -Wno-strict-aliasing 
#-Wno-implicit-function-declaration 
CPPFLAGS+=-std=c++0x -I./ -I./include -I./hik -Wl,--hash-style=sysv
LDFLAGS=-lpthread -lrt -ldl -lstdc++

ifeq ($(ARCH),pc)
LIBNAME=libpcits
CXX=g++ -m32
AR=ar
else
LIBNAME=libits
CXX=arm-none-linux-gnueabi-g++
AR=arm-none-linux-gnueabi-ar
endif
HIKOBJS += main.o 
HIK = communication.o countdown.o download.o upload.o extconfig.o hiktsc.o sqlite_conf.o chanlockcontrol.o
HIKOBJS += $(patsubst %.o,./hik/%.o,$(HIK))

ifeq ($(ARCH),pc)
SLIB += ./libits/libpcits.a
CPPFLAGS += -DFOR_SERVER -Wno-unused-function -Wno-unused-parameter -Wno-sign-compare -Wno-write-strings
LDFLAGS += -lsqlite3
else
SLIB += ./libits/libits.a 
	ifeq ($(USELIB),d)
	LDFLAGS += -L./ -lsqlite3
	else
	SLIB += ./libsqlite3.a
	endif
endif

all: clean hiktsc
	$(RM) *.o $(HIKOBJS)

hiktsc: $(HIKOBJS) $(SLIB)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CPPFLAGS)
	
clean:
	$(RM) $(HIKOBJS)

