CFLAGS=-Wall -O3 -fPIC

ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

ifeq ($(ARCH),pc)
CC=gcc
AR=ar
SLIB=libpchikconfig.a
DLIB=libpchikconfig.so
NAME_PARSE_INI_LIB=libpcparseini.a
else
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
SLIB=libhikconfig.a
DLIB=libhikconfig.so
NAME_PARSE_INI_LIB=libparseini.a
endif
#LFLAGS=-Wl,rpath=$(PWD)

OBJS=HikConfig.o write_config_file.o read_config_file.o
OBJS+=$(shell $(AR) -t $(NAME_PARSE_INI_LIB))

all: clean xlib $(SLIB) $(DLIB)
	$(RM) *.o

xlib:
	$(AR) -x $(NAME_PARSE_INI_LIB)

$(SLIB): $(OBJS)
	$(AR) crs $@ $^ 

$(DLIB): $(OBJS)
	$(CC) -shared $^ -o $@

clean:
	$(RM) *.o $(SLIB) $(DLIB)
