CFLAGS=-Wall -O3 -fPIC -I../../common/ -I../parse_ini/ -I../../include

ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

ifeq ($(ARCH),pc)
CC=gcc
AR=ar
SLIB=libpcgbconfig.a
DLIB=libpcgbconfig.so
NAME_PARSE_INI_LIB=../parse_ini/libpcparseini.a
else
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
SLIB=libgbconfig.a
DLIB=libgbconfig.so
NAME_PARSE_INI_LIB=../parse_ini/libparseini.a
endif
#LFLAGS=-Wl,rpath=$(PWD)

#如果parse_ini库不存在，则运行其所在的makefile
ifneq ($(NAME_PARSE_INI_LIB),$(wildcard $(NAME_PARSE_INI_LIB)))
TARGET+=$(NAME_PARSE_INI_LIB)
endif

ALL_SOURCE=$(wildcard *.c)
OBJS+=$(ALL_SOURCE:.c=.o)
OBJS+=$(shell $(AR) -t $(NAME_PARSE_INI_LIB))

TARGET+=clean xlib $(SLIB) $(DLIB)

all: $(TARGET)
	$(RM) *.o

xlib:
	$(AR) -x $(NAME_PARSE_INI_LIB)

$(SLIB): $(OBJS)
	$(AR) crs $@ $^ 

$(DLIB): $(OBJS)
	$(CC) -shared $^ -o $@

$(NAME_PARSE_INI_LIB):
	make -C ../parse_ini/

clean:
	$(RM) *.o $(SLIB) $(DLIB)
