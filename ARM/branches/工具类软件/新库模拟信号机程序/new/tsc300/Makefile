ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm")
ARCH=arm
endif

OPTIMIZER = -g #-DDEBUG
CFLAGS = $(OPTIMIZER) -Wall -Werror -unaligned_access -Wno-implicit-function-declaration -Wno-unused-function -Wno-unused-variable -Wno-array-bounds -Wno-strict-aliasing -Wl,--hash-style=sysv
CFLAGS+= -I../include -I../libs/hikconfig/ -I../libs/parse_ini/ -I../libs/logsystem/
#信号机软件程序版本硬件版本,长度控制在32个字符以内，否则sadp获取会异常
DATE=$(shell date +%Y.%m.%d)
SOFTWARE_VERSION="HIKETSC-V2.0.0.0-$(DATE)"
HARDWARE_VERSION="DS-TSC300"
CFLAGS+=-DSOFTWARE_VERSION_INFO=\"$(SOFTWARE_VERSION)\"
CFLAGS+=-DHARDWARE_VERSION_INFO=\"$(HARDWARE_VERSION)\"
LDFLAGS = -lpthread -lrt -ldl -lm

PGM = hikTSC300
PGMOBJS = main.o io.o canmsg.o 
COMMON = initcommon.o inifile.o binfile.o configureManagement.o \
         datacollect.o download.o upload.o communication.o \
		 faultcheck.o lcb.o \
		 countDown.o countDown_Hisense.o countDown_LaiSi.o countDown_Nation.o countDown_Pulse.o
PGMOBJS += $(patsubst %.o,../common/%.o,$(COMMON))

ifeq "$(origin USELIB)" "undefined"
$(warning "You can run make USELIB=s or USELIB=d, which decide to use static lib or dynamic lib")
USELIB=s
endif

ifeq ($(ARCH),arm)
CC = arm-none-linux-gnueabi-gcc
AR = arm-none-linux-gnueabi-ar
SLIB += ../libs/libsocketcan.a ../libs/hikconfig/libhikconfig.a

	ifeq ($(USELIB),d)
	LDFLAGS += -L../libits -lits
	else
	SLIB += ../libits/libits.a
	endif

else
CC = gcc
AR = ar
SLIB += ../libs/hikconfig/libpchikconfig.a
CFLAGS += -DFOR_SERVER
	ifeq ($(USELIB),d)
	LDFLAGS += -L../libits -lpcits
	else
	SLIB += ../libits/libpcits.a
	endif

endif

all: clean $(PGM)
	$(RM) $(PGMOBJS)

$(PGM): $(PGMOBJS) $(SLIB)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) *.o $(PGM) $(PGMOBJS) 



