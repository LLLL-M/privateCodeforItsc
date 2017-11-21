ifeq ($(shell uname), Linux)
#linux,首先判断到底是使用arm平台还是pclinux的x86平台
ifeq "$(origin ARCH)" "undefined"
$(warning "You can run make ARCH=pc or ARCH=arm ARCH=armF2, default[ARCH=armF2]")
ARCH = armF2
endif

ifeq ($(ARCH),arm)
#CXX = arm-F2-4.9-linux-gnueabihf-g++
CXX = arm-none-linux-gnueabi-g++
LIBDIR = ../libforarmlinux
LIBSOCKCAN = ../libforarmlinux/libsocketcan.a
else
ifeq ($(ARCH),armF2)
CXX = arm-F2-4.9-linux-gnueabihf-g++
LIBDIR = ../libforarmF2
LIBSOCKCAN = ../libforarmF2/libsocketcan.a
else
CXX = g++-7.1.0
LIBDIR = ../libforpclinux
endif
endif
#CXXFLAGS = 
LDFLAGS = -lpthread -lrt

TARGET = TSC300_V3.0.0.0

else
#windows
#LDFLAGS = -lwinmm
LIBDIR = ../libforwindows
RM = del
CXX = g++
TARGET = ../libforwindows/TSC300_V3
endif

CXXFLAGS += -I../include -I../zmqinclude -std=c++11 -Wall -O3 -DHARDWARE_MODEL=300 #-DDEBUG#-Wno-sign-compare #-Wno-unused-variable
LDFLAGS += -L$(LIBDIR) -lzmq -lczmq -latomic -ljson -lunqlite

SRC:=$(wildcard *.cpp)
SRC+=$(wildcard ../common/*.cpp)
OBJS:=$(SRC:.cpp=.o)


.PHONY: $(TARGET) clean

all: $(TARGET)
	$(RM) $(OBJS)

$(TARGET): $(OBJS) $(LIBSOCKCAN)
	$(CXX) -s $^ -o $@ $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)
