ifeq "$(origin ARCH)" "undefined"
$(warning You can run 'make ARCH=pc or ARCH=arm')
ARCH=pc
endif

OPTIMIZER=-fPIC
#CPPFLAGS=$(OPTIMIZER) -Wall -Werror -unaligned_access -Wno-unused-variable -Wno-array-bounds -Wno-strict-aliasing 
#-Wno-implicit-function-declaration 
CPPFLAGS+=-std=c++0x -I./ -I../include -Wl,--hash-style=sysv
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
 
SRC:=$(wildcard *.cpp)
LIBOBJS:=$(SRC:.cpp=.o)

DLIB=$(LIBNAME).so
SLIB=$(LIBNAME).a

all: clean $(SLIB) $(DLIB)
	$(RM) *.o

$(SLIB): $(LIBOBJS)
	$(AR) crs $@ $(LIBOBJS)

$(DLIB): $(LIBOBJS)
	$(CXX) -shared $(LDFLAGS) $^ -o $@ #-Wl,-soname,$(LIBNAME).so.$(VER)

clean:
	$(RM) *.o $(SLIB) $(DLIB)

