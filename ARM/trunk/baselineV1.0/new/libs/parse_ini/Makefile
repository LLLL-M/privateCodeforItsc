CFLAGS=-Wall -O3 -fPIC

ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

ifeq ($(ARCH),pc)
CC=gcc
AR=ar
LIBNAME=libpcparseini
else
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
LIBNAME=libparseini
endif
#LFLAGS=-Wl,rpath=$(PWD)

VER=0.0.1
DLIB=$(LIBNAME).so
SLIB=$(LIBNAME).a
all: clean $(SLIB) $(DLIB) test
	$(RM) *.o

test: test.o $(SLIB)
	$(CC) $^ -o $@ 

test.o:

$(SLIB): parse_ini.o
	$(AR) crs $@ $^ 

$(DLIB): parse_ini.o
	$(CC) -shared $^ -o $@ #-Wl,-soname,$(LIBNAME).so.$(VER)

parse_ini.o:

clean:
	$(RM) *.o test $(SLIB) $(DLIB)
