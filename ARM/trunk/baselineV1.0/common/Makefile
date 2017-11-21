CROSS_COMPILE ?= arm-none-linux-gnueabi-
CFLAGS=-s -unaligned_access -Wall -Wno-implicit-function-declaration -O3 #-DDEBUG -Wno-array-bounds 
LDFLAGS=-lpthread
CFLAGS+=-I../common/ -I../libs/hikconfig/ -I../libs/parse_ini/ -I../libs/LogSystem/
SLIB=../libs/hikconfig/libhikconfig.a ../libs/libits.a ../libs/libsocketcan.a

#考虑到udpserver.c里面用到了版本信息这个宏，该宏是通过tsc300或tsc500传递的，故这里编译的时候就忽略了错误
MAKEFLAGS=k

#通过这种方式，可以将本目录下面的所有.c文件编译成.o文件
ALL_SOURCE:=$(wildcard *.c)
ALL_SOURCE:=$(ALL_SOURCE:.c=.o)

CC = $(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

all: $(ALL_SOURCE)

clean:
	$(RM) *.o

