CFLAGS=-Wall -O3 -fPIC -I./ -I../../include/ -I../../common/ -I../../libs/sqlite3_conf/ 

ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

ifeq ($(ARCH),pc)
CC=gcc -m32
AR=ar
SLIB=libpchikconfig.a
DLIB=libpchikconfig.so
NAME_SQLITE_LIB=../../libs/sqlite3_conf/libpcsqliteconf.a
else
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
SLIB=libhikconfig.a
DLIB=libhikconfig.so
NAME_SQLITE_LIB=../../libs/sqlite3_conf/libsqliteconf.a
endif
#LFLAGS=-Wl,rpath=$(PWD)

#如果parse_ini库不存在，则运行其所在的makefile
ifneq ($(NAME_SQLITE_LIB),$(wildcard $(NAME_SQLITE_LIB)))
TARGET+=$(NAME_SQLITE_LIB)
endif

ALL_SOURCE=$(wildcard *.c)
OBJS+=$(ALL_SOURCE:.c=.o)
OBJS+=$(shell $(AR) -t $(NAME_SQLITE_LIB))

TARGET+=clean xlib $(SLIB) $(DLIB)

all: $(TARGET)
	$(RM) *.o

xlib:
	$(AR) -x $(NAME_SQLITE_LIB)

$(SLIB): $(OBJS)
	$(AR) crs $@ $^ 

$(DLIB): $(OBJS)
	$(CC) -shared $^ -o $@

#$(NAME_SQLITE_LIB):
	

clean:
	$(RM) *.o $(SLIB) $(DLIB) 
