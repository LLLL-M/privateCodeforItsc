ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

#OPTIMIZER = -g #-DDEBUG
CFLAGS = $(OPTIMIZER) -Wall -Werror -unaligned_access -Wno-implicit-function-declaration -Wno-unused-variable -Wno-array-bounds -Wno-strict-aliasing -Wl,--hash-style=sysv
CFLAGS+= -I../../include -I../hikconfig/ -I../../libs/parse_ini/ -I../../libs/logsystem/ -I../../libs/mxml/ -I../../libs/sqlite3_conf/
#信号机软件程序版本硬件版本,长度控制在32个字符以内，否则sadp获取会异常
ifeq "$(origin VER)" "undefined"
VER=$(shell date +%Y.%m.%d)
endif
SOFTWARE_VERSION="HIKETSC-V2.1.0.0-$(VER)"
HARDWARE_VERSION="DS-TSC300"
CFLAGS+=-DSOFTWARE_VERSION_INFO=\"$(SOFTWARE_VERSION)\"
CFLAGS+=-DHARDWARE_VERSION_INFO=\"$(HARDWARE_VERSION)\"
LDFLAGS = -lpthread -lrt -ldl -lm

PGMOBJS = main.o io.o canmsg.o
COMMON = initcommon.o configureManagement.o can.o camera.o busdetector.o\
         datacollect.o download.o upload.o communication.o \
		 faultcheck.o lcb.o hik.o chancontrol.o common.o mxmlhandle.o \
		 countDown.o countDown_Hisense.o countDown_LaiSi.o countDown_Nation.o countDown_Pulse.o \
		 countdown_nation_2004.o
PGMOBJS += $(patsubst %.o,../common/%.o,$(COMMON))

ifeq "$(origin USELIB)" "undefined"
$(warning "You can run make USELIB=s or USELIB=d, which decide to use static lib or dynamic lib")
USELIB=d
endif

ifeq ($(ARCH),arm)
PGM = hikTSC300
CC = arm-none-linux-gnueabi-gcc
AR = arm-none-linux-gnueabi-ar
SLIB += ../../libs/libsocketcan.a ../hikconfig/libhikconfig.a ../../libs/mxml/libhikxml.a ../../libs/sqlite3_conf/libsqliteconf.a ../../libits/libits.a 
		

	ifeq ($(USELIB),d)
	LDFLAGS += -L../../libs -lsqlite3
	else
	SLIB += ../../libs/libsqlite3.a
	endif

else
PGM = hikpcTSC300
CC = gcc -m32
AR = ar
SLIB += ../hikconfig/libpchikconfig.a ../../libs/mxml/libpchikxml.a ../../libs/sqlite3_conf/libpcsqliteconf.a ../../libits/libpcits.a
CFLAGS += -Wno-unused-function
LDFLAGS += -lsqlite3

endif

all: clean $(PGM)
	$(RM) $(PGMOBJS)

$(PGM): $(PGMOBJS) $(SLIB)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) *.o $(PGM) $(PGMOBJS) 



