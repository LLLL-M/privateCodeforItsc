CFLAGS=-Wall -O3 -fPIC

ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

ifeq ($(ARCH),pc)
CC=gcc -m32
AR=ar
LIBNAME=libpclogsystem
else
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
LIBNAME=liblogsystem
endif


VER=0.0.1
DLIB=$(LIBNAME).so
SLIB=$(LIBNAME).a
all: clean $(SLIB) $(DLIB) test
	$(RM) *.o

test: test.o $(SLIB)
	$(CC) $^ -o $@ 

test.o:

$(SLIB): LogSystem.o
	$(AR) crs $@ $^ 

$(DLIB): LogSystem.o
	$(CC) -shared $^ -o $@ 

LogSystem.o:

clean:
	$(RM) *.o test $(SLIB) $(DLIB)
