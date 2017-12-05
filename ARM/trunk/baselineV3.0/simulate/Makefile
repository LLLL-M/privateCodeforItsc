SRC:=$(wildcard ../tsc/*.cpp)
SRC+=$(wildcard ../common/*.cpp)
OBJS:=$(SRC:.cpp=.o)

ifeq ($(shell uname), Linux)
#linux,首先判断到底是使用arm平台还是pclinux的x86平台
CXX = g++-7.1.0
LIBDIR = ../libforpclinux
LDFLAGS = -pthread -lrt
CXXFLAGS = -I../tsc -I./
DELOBJ = $(OBJS)
else
#windows
#LDFLAGS = -lwinmm
LIBDIR = ../libforwindows
RM = del
CXX = g++
#因为windows默认头文件包含当前路径为main.cpp所在路径，所以即使当前处理simulate目录，但实际编译时头文件包含的当前路径为../tsc/
CXXFLAGS = -I../simulate -I../tsc
#因为windows的删除路径与linux不一致，路径使用反斜杠(\)，而linux使用正斜杠(/)
DELOBJ = ..\common\*.o ..\tsc\*.o
endif

TARGET = hikTSC_V3
CXXFLAGS += -I../include -I../include/zmq
CXXFLAGS += -std=c++11 -Wall -O3 #-DDEBUG#-Wno-sign-compare #-Wno-unused-variable
LDFLAGS += -L$(LIBDIR) -lzmq -lczmq -latomic -ljson -lunqlite




.PHONY: $(TARGET) clean

all: $(TARGET)
	$(RM) $(DELOBJ)

$(TARGET): $(OBJS) 
	$(CXX) -s $^ -o $@ $(LDFLAGS)

clean:
	$(RM) $(DELOBJ) $(TARGET)
