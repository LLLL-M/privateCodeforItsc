TARGET = hikTSC300
CXX = arm-at91-linux-gnueabi-g++
LIBDIR = ../libforat91
LIBSOCKCAN = $(LIBDIR)/libsocketcan.a
CXXFLAGS += -I../include -I../include/zmq -I../tsc -I./ -std=c++11 -Wall -O3 #-DDEBUG#-Wno-sign-compare #-Wno-unused-variable
LDFLAGS = -pthread -lrt -latomic
LDFLAGS += -L$(LIBDIR) -lzmq -lczmq -ljson -lunqlite

SRC := $(wildcard ../tsc/*.cpp)
SRC += $(wildcard ../common/*.cpp)
SRC += $(wildcard ./*.cpp)
OBJS:=$(SRC:.cpp=.o)


.PHONY: $(TARGET) clean

all: $(TARGET)
	$(RM) $(OBJS)

$(TARGET): $(OBJS) $(LIBSOCKCAN)
	$(CXX) -s $^ -o $@ $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)
